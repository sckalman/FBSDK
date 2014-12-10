/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include "common.h"
#include "ui.h"
#include "screen_ui.h"
#include "device.h"
#include "bootloader.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "roots.h"
#include "rkimage.h"
#include "mtdutils/mounts.h"
#include "mtdutils/rk29.h"
#include "mtdutils/mtdutils.h"



static const char *CACHE_ROOT = "/cache";
static char IN_SDCARD_ROOT[64] = "\0";
static char EX_SDCARD_ROOT[64] = "\0";

bool bNeedClearMisc = true;
bool bIfUpdateLoader = false;
char gVolume_label[64];

static int
erase_volume(const char *volume) {
    ui_set_background(BACKGROUND_ICON_INSTALLING);
    ui_show_indeterminate_progress();
    ui_print("Formatting %s...\n", volume);

    ensure_path_unmounted(volume);

    return format_volume(volume);
}


static char**
prepend_title(const char** headers) {
    char* title[] = { "Android system recovery <"
                          EXPAND(RECOVERY_API_VERSION) "e>",
                      "",
                      NULL };

    // count the number of lines in our title, plus the
    // caller-provided headers.
    int count = 0;
    char** p;
    for (p = title; *p; ++p, ++count);
    for (p = headers; *p; ++p, ++count);

    char** new_headers = malloc((count+1) * sizeof(char*));
    char** h = new_headers;
    for (p = title; *p; ++p, ++h) *h = *p;
    for (p = headers; *p; ++p, ++h) *h = *p;
    *h = NULL;

    return new_headers;
}

static int
get_menu_selection(char** headers, char** items, int menu_only,
                   int initial_selection) {
    // throw away keys pressed previously, so user doesn't
    // accidentally trigger menu items.
    ui_clear_key_queue();

    ui_start_menu(headers, items, initial_selection);
    int selected = initial_selection;
    int chosen_item = -1;

    while (chosen_item < 0) {
        int key = ui_wait_key();
        int visible = ui_text_visible();

        if (key == -1) {   // ui_wait_key() timed out
            if (ui_text_ever_visible()) {
                continue;
            } else {
                LOGI("timed out waiting for key input; rebooting.\n");
                ui_end_menu();
                return ITEM_REBOOT;
            }
        }

        int action = device_handle_key(key, visible);

        if (action < 0) {
            switch (action) {
                case HIGHLIGHT_UP:
                    --selected;
                    selected = ui_menu_select(selected);
                    break;
                case HIGHLIGHT_DOWN:
                    ++selected;
                    selected = ui_menu_select(selected);
                    break;
                case SELECT_ITEM:
                    chosen_item = selected;
                    break;
                case NO_ACTION:
                    break;
            }
        } else if (!menu_only) {
            chosen_item = action;
        }
    }

    ui_end_menu();
    return chosen_item;
}


static void
wipe_data(int confirm) {
    if (confirm) {
        static char** title_headers = NULL;

        if (title_headers == NULL) {
            char* headers[] = { "Confirm wipe of all user data?",
                                "  THIS CAN NOT BE UNDONE.",
                                "",
                                NULL };
            title_headers = prepend_title((const char**)headers);
        }

        char* items[] = { " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " Yes -- delete all user data",   // [7]
                          " No",
                          " No",
                          " No",
                          NULL };

        int chosen_item = get_menu_selection(title_headers, items, 1, 0);
        if (chosen_item != 7) {
            return;
        }
    }

    ui_print("\n-- Wiping data...\n");
    device_wipe_data();
    erase_volume("/data");
    erase_volume("/cache");
    ui_print("Data wipe complete.\n");
}

static void
prompt_and_wait() {
    char** headers = prepend_title((const char**)MENU_HEADERS);

    for (;;) {
        ui_reset_progress();

        int chosen_item = get_menu_selection(headers, MENU_ITEMS, 0, 0);

        // device-specific code may take some action here.  It may
        // return one of the core actions handled in the switch
        // statement below.
        chosen_item = device_perform_action(chosen_item);

        int status;
        int wipe_cache;
        switch (chosen_item) {
            case ITEM_REBOOT:
                return;

            case ITEM_WIPE_DATA:
                wipe_data(ui_text_visible());
                if (!ui_text_visible()) return;
                break;

            case ITEM_WIPE_CACHE:
                ui_print("\n-- Wiping cache...\n");
                erase_volume("/cache");
                ui_print("Cache wipe complete.\n");
                if (!ui_text_visible()) return;
                break;
        }
    }
}

static void
print_property(const char *key, const char *name, void *cookie) {
    printf("%s=%s\n", key, name);
}

void SetSdcardRootPath(void) {
     property_get("InternalSD_ROOT", IN_SDCARD_ROOT, "");
	 LOGI("InternalSD_ROOT: %s\n", IN_SDCARD_ROOT);
	 property_get("ExternalSD_ROOT", EX_SDCARD_ROOT, "");
	 LOGI("ExternalSD_ROOT: %s\n", EX_SDCARD_ROOT);
}

void SureCacheMount() {
	if(ensure_path_mounted("/cache")) {
		printf("mount cache fail,so formate...\n");
		format_volume("/cache");
		ensure_path_mounted("/cache");
	}
}

ssize_t mygetline(char **lineptr, size_t *n, FILE *stream) {
	if(*n <= 0) {
		*lineptr = malloc(128);
		memset(*lineptr, 0, 128);
		*n = 128;
	}

	char c;
	char *pline = *lineptr;
	size_t count = 0;
	while((fread(&c, 1, 1, stream)) == 1) {
		if(c == '\n') {
			*pline = '\0';
			return count;
		}else if(c == '\r') {
			fread(&c, 1, 1, stream);
			if(c == '\n'){
				*pline = '\0';
				return count;
			}
		}else {
			if(count >= *n -1) {
				*lineptr = realloc(*lineptr, *n + 128);
				*n = *n + 128;
				pline = *lineptr + count;
			}

			*pline = c;
			count++;
			pline++;
		}
	}

	*pline = '\0';
	if(count == 0) {
		return -1;
	}

	return count;
}

char* readConfig(FILE *pCfgFile, char const *keyName) {
	char *keyValue = NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;

	fseek(pCfgFile, 0, SEEK_SET);
	while((read = mygetline(&line, &len, pCfgFile)) != -1) {

		char *pstr = line;
		if(*pstr == '#' || *pstr == '\0') {
			continue;
		}
		printf("get line %s\n", pstr);
		if(strstr(pstr, keyName) != NULL) {
			char *pValue = strchr(pstr, '=');
			if(pValue != NULL) {
				keyValue = malloc(strlen(pValue) + 1);
				while(*(++pValue) == ' ');
				strcpy(keyValue, pValue);
				printf("find property %s value %s\n", keyName, keyValue);
				break;
			}
		}
	}

	printf("read config end\n");
	free(line);
	return keyValue;
}

void parseSDBootConfig() {
	char configPath[64];
	strcpy(configPath, EX_SDCARD_ROOT);
	strcat(configPath, "/sd_boot_config.config");

	printf("sd boot config file is %s", configPath);
	FILE* configFile = fopen(configPath, "rb");

	if(!configFile) {
		printf("no find config file!\n");
		return;
	}

	char *pValue;
	if((pValue = readConfig(configFile, SdBootConfigs[pcba_test].name)) != NULL) {
		printf("set pcba_test value is %s\n", pValue);
		SdBootConfigs[pcba_test].value = pValue;
	}

	if((pValue = readConfig(configFile, SdBootConfigs[fw_update].name)) != NULL) {
		printf("set fw_update value is %s\n", pValue);
		SdBootConfigs[fw_update].value = pValue;
	}

	if((pValue = readConfig(configFile, SdBootConfigs[display_led].name)) != NULL) {
		printf("set display_led value is %s\n", pValue);
		SdBootConfigs[display_led].value = pValue;
	}

	if((pValue = readConfig(configFile, SdBootConfigs[display_lcd].name)) != NULL) {
		printf("set display_lcd value is %s\n", pValue);
		SdBootConfigs[display_lcd].value = pValue;
	}

	if((pValue = readConfig(configFile, SdBootConfigs[demo_copy].name)) != NULL) {
		printf("set demo_copy value is %s\n", pValue);
		SdBootConfigs[demo_copy].value = pValue;
	}

	if((pValue = readConfig(configFile, SdBootConfigs[volume_label].name)) != NULL) {
		printf("set volume_label value is %s\n", pValue);
		SdBootConfigs[volume_label].value = pValue;
		strcpy(gVolume_label, SdBootConfigs[volume_label].value);
	}else {
		property_get("UserVolumeLabel", gVolume_label, SdBootConfigs[volume_label].value);
	}
}

void checkSDRemoved() {

	while(1) {
		int value = access("/dev/block/mmcblk0", 0);
		if(value == -1) {
			printf("remove sdcard\n");
			break;
		}else {
			sleep(1);
		}
	}
}


int
main(int argc, char **argv) {
    time_t start = time(NULL);

    freopen("/dev/ttyFIQ0", "a", stdout); setbuf(stdout, NULL);
    freopen("/dev/ttyFIQ0", "a", stderr); setbuf(stderr, NULL);

    //freopen("/dev/ttyS1", "a", stdout); setbuf(stdout, NULL);
    //freopen("/dev/ttyS1", "a", stderr); setbuf(stderr, NULL);


    printf("Starting recovery on %s", ctime(&start));

    load_volume_table();
    mtd_scan_partitions();
	SetSdcardRootPath();

    get_args(&argc, &argv);
    char *factory_mode = NULL;
    int arg;
    while ((arg = getopt_long(argc, argv, "", OPTIONS, NULL)) != -1) {
        switch (arg) {
        case 'f': factory_mode = optarg; break;
        case '?':
            LOGE("Invalid command argument\n");
            continue;
        }
    }

	SureCacheMount();
	int i;
	for(i = 0; i < 1; i++) {
		if(0 == ensure_path_mounted(EX_SDCARD_ROOT)){
			break;
		}else {
			printf("delay 2sec\n");
			sleep(2);
		}
	}

	ensure_path_mounted(IN_SDCARD_ROOT);
	parseSDBootConfig();
	int status = INSTALL_SUCCESS;
	bool pcbaTestPass = true;

	if(!strcmp(SdBootConfigs[pcba_test].value, "1")) {
		//pcba test
		printf("enter pcba test!\n");

		char *args[2];
		args[0] = "/sbin/pcba_core";
		args[1] = NULL;

		pid_t child = fork();
		if (child == 0) {
			execv(args[0], args);
			fprintf(stderr, "run_program: execv failed: %s\n", strerror(errno));
			status = INSTALL_ERROR;
			pcbaTestPass = false;
		}
		int child_status;
		waitpid(child, &child_status, 0);
		if (WIFEXITED(child_status)) {
			if (WEXITSTATUS(child_status) != 0) {
				printf("pcba test error coder is %d \n", WEXITSTATUS(child_status));
				status = INSTALL_ERROR;
				pcbaTestPass = false;
			}
		} else if (WIFSIGNALED(child_status)) {
			printf("run_program: child terminated by signal %d\n", WTERMSIG(child_status));
			status = INSTALL_ERROR;
			pcbaTestPass = false;
		}
	}

    device_ui_init(&ui_parameters);
    ui_init();
    ui_print("sdcard boot tools system v1.34 \n\n");
    //ui_set_background(BACKGROUND_ICON_INSTALLING);

    if(!pcbaTestPass) {
    	ui_print("pcba test error!!!");
    	goto finish;
    }

	//format user partition
	erase_volume(IN_SDCARD_ROOT);
	//format userdata
	erase_volume("/data");

	if(!strcmp(SdBootConfigs[fw_update].value, "1")) {
		//fw update
		char *updateImagePath = malloc(100);
		strcpy(updateImagePath, EX_SDCARD_ROOT);
		strcat(updateImagePath, "/sdupdate.img");

		status = install_rkimage(updateImagePath);
		if(status != INSTALL_SUCCESS) {
			goto finish;
		}
	}

	if(!strcmp(SdBootConfigs[demo_copy].value, "1")) {

		if(ensure_path_mounted(IN_SDCARD_ROOT)) {
			printf("mount user partition error!\n");
			goto finish;
		}

		//copy demo files
		char *demoPath = malloc(strlen(EX_SDCARD_ROOT) + 64);

		strcpy(demoPath, EX_SDCARD_ROOT);
		strcat(demoPath, "/Demo");

		char *args[6];
		args[0] = "/sbin/busybox";
		args[1] = "cp";
		args[2] = "-R";
		args[3] = demoPath;
		args[4] = IN_SDCARD_ROOT;
		args[5] = NULL;

		pid_t child = fork();
		if (child == 0) {
			printf("run busybox copy demo files...\n");
			execv(args[0], &args[1]);
			fprintf(stderr, "run_program: execv failed: %s\n", strerror(errno));
			_exit(1);
		}
		int child_status;
		waitpid(child, &child_status, 0);
		if (WIFEXITED(child_status)) {
			if (WEXITSTATUS(child_status) != 0) {
				fprintf(stderr, "run_program: child exited with status %d\n",
						WEXITSTATUS(child_status));
			}
		} else if (WIFSIGNALED(child_status)) {
			fprintf(stderr, "run_program: child terminated by signal %d\n",
					WTERMSIG(child_status));
		}

		free(demoPath);
	}

finish:
    if (status != INSTALL_SUCCESS) ui_set_background(BACKGROUND_ICON_ERROR);
    if (status != INSTALL_SUCCESS) {
    	bNeedClearMisc = false;
    	ui_show_text(1);
        prompt_and_wait();
    }else {
    	if((factory_mode != NULL && !strcmp(factory_mode, "small")) || bIfUpdateLoader == true) {
    		printf("small fw ,or bIfUpdateLoader = %d\n", bIfUpdateLoader);
    		bNeedClearMisc = false;
    	}
    }

    // Otherwise, get ready to boot the main system...
    finish_recovery(NULL);
    ui_show_text(1);
    ui_print("All complete successful!please remove the sdcard......\n");
    checkSDRemoved();
    ui_print("rebooting...\n");
    android_reboot(ANDROID_RB_RESTART, 0, 0);
    return EXIT_SUCCESS;
}
