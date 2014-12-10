/*
 * custom.c
 *
 *  Created on: 2013-4-27
 *      Author: mmk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parse_xml.h"
#include "board_id_ctrl.h"

static int run(const char *filename, char *const argv[])
{
    struct stat s;
    int status;
    pid_t pid;

    if (stat(filename, &s) != 0) {
        fprintf(stderr, "cannot find '%s'", filename);
        return -1;
    }

    pid = fork();
    if (pid == 0) {
        setpgid(0, getpid());
        /* execute */
        execv(filename, argv);
        fprintf(stderr, "can't run %s (%s)\n", filename, strerror(errno));
        /* exit */
        _exit(0);
    }

    if (pid < 0) {
        fprintf(stderr, "failed to fork and start '%s'\n", filename);
        return -1;
    }

    if (-1 == waitpid(pid, &status, WCONTINUED | WUNTRACED)) {
        fprintf(stderr, "wait for child error\n");
        return -1;
    }

    printf("executed '%s' return %d\n", filename, WEXITSTATUS(status));
    return 0;
}

static int setProp(char *name, char *value) {
	char buf[128];
	char *cmd[6];
	int length;

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "/%s/d", name);
	cmd[0] = "/sbin/busybox";
	cmd[1] = "sed";
	cmd[2] = "-i";
	cmd[3] = buf;
	cmd[4] = "system/build.prop";
	cmd[5] = NULL;
	printf("%s %s %s %s %s \n", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
	run(cmd[0], cmd);

	FILE *f = fopen("system/build.prop", "a");
	if(f == NULL) {
		printf("open system/build.prop error==========> \n");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "\n%s=%s", name, value);
	length = strlen(name) + strlen(value) + 2;
	//fseek(f, 0, SEEK_END);
	if(length != fwrite(buf, 1, length, f)) {
		printf("write prop error =============\n");
		fclose(f);
		return -1;
	}

	printf("write success\n");
	fclose(f);
	return 0;
}

void customHandler(char *command, int argc, char **argv) {
	char *cmd[10];
	char *str;
	char *str1;
	char str3[128];

	if(!strcmp(command, "cp") || !strcmp(command, "CP")) {
		str = strrchr(argv[1], '/');
		str1 = strndup(argv[1], str - argv[1]);
		strcpy(str3, "cust/backup/");
		strcat(str3, str1);
		printf("mkdir %s \n", str3);
		//mkdir des target
		cmd[0] = "/sbin/busybox";
		cmd[1] = "mkdir";
		cmd[2] = "-p";
		cmd[3] = str3;
		cmd[4] = NULL;
		run(cmd[0], cmd);

		//backup des target
		cmd[1] = "cp";
		cmd[2] = "-a";
		cmd[3] = argv[1];
		cmd[4] = str3;
		cmd[5] = NULL;
		run(cmd[0], cmd);

		//cp src to des
		cmd[3] = argv[0];
		cmd[4] = argv[1];
		cmd[5] = NULL;
		run(cmd[0], cmd);

		//chmod des
		cmd[1] = "chmod";
		cmd[2] = argv[2];
		cmd[3] = argv[1];
		cmd[4] = NULL;
		run(cmd[0], cmd);

		free(str1);
	}else if(!strcmp(command, "rm") || !strcmp(command, "RM")) {
		str = strrchr(argv[0], '/');
		str1 = strndup(argv[0], str - argv[0]);
		strcpy(str3, "cust/backup/");
		strcat(str3, str1);
		printf("mkdir %s \n", str3);
		//mkdir des target
		cmd[0] = "/sbin/busybox";
		cmd[1] = "mkdir";
		cmd[2] = "-p";
		cmd[3] = str3;
		cmd[4] = NULL;
		run(cmd[0], cmd);

		//backup src target
		cmd[1] = "cp";
		cmd[2] = "-a";
		cmd[3] = argv[0];
		cmd[4] = str3;
		cmd[5] = NULL;
		run(cmd[0], cmd);

		//rm src
		cmd[1] = "rm";
		cmd[2] = argv[0];
		cmd[3] = NULL;
		run(cmd[0], cmd);

		free(str1);
	}else if(!strcmp(command, "set") || !strcmp(command, "SET")) {
		setProp(argv[0], argv[1]);
	}
}


int saveBoardIdToFile(char *board_id) {
	FILE *fp_id;
	fp_id = fopen("cust/last_board_id", "w");
	if(fp_id == NULL) {
		printf("open last_board_id error\n");
		return -1;
	}

	int length = DEVICE_NUM_TYPES;
	printf("length:%d\n", length);
	int i;
	for(i = 0; i < length; i++) {
		printf("board-id: %d \n", *(board_id+i));
	}

	if(1 != fwrite(&length, 4, 1, fp_id)) {
		printf("write length error1\n");
		fclose(fp_id);
		return -1;
	}

	if(length != fwrite(board_id, 1, length, fp_id)) {
		printf("write board id error!\n");
		fclose(fp_id);
		return -1;
	}

	printf("save board id success!\n");
	fclose(fp_id);
	return 0;
}

int backupProp() {
	char *cmd[10];

	cmd[0] = "/sbin/busybox";
	cmd[1] = "mkdir";
	cmd[2] = "-p";
	cmd[3] = "cust/backup/system";
	cmd[4] = NULL;
	run(cmd[0], cmd);

	cmd[1] = "cp";
	cmd[2] = "-a";
	cmd[3] = "system/build.prop";
	cmd[4] = "cust/backup/system/build.prop";
	cmd[5] = NULL;
	run(cmd[0], cmd);

	printf("backup build.prop complete!\n");
	return 0;
}

int custom() {

	char area[32];
	char language[32];
	char local[32];	
	char geo[32];	
	char timezone[32];	
	char user_define[32];
	
	char operator[32];
	char reserve[32];
	FILE *fp_area;

	printf("*********** start custom ***************\n");
	board_id_open_device();

	//get language from ioctrl
	memset(area, 0, sizeof(area));
	memset(language, 0, sizeof(language));
	memset(local, 0, sizeof(local));	
	memset(geo, 0, sizeof(geo));
	memset(timezone, 0, sizeof(timezone));
	memset(user_define, 0, sizeof(user_define));
	
	memset(operator, 0, sizeof(operator));
	memset(reserve, 0, sizeof(reserve));

	board_id_get_operator_name(DEVICE_TYPE_OPERATOR, local, operator);
	board_id_get_reserve_name(DEVICE_TYPE_RESERVE, local, reserve);
	board_id_get_locale_region(DEVICE_TYPE_AREA, area, language, local, geo, timezone, user_define);
	printf("get area form ioctrl: area=%s, language=%s, local=%s, timezone=%s, operator=%s, reserve=%s\n",
				area, language, local, timezone, operator, reserve);


	fp_area = fopen("/cust/cust.xml", "r");
	if(fp_area == NULL) {
		printf("open area.xml error!\n");
		board_id_close_device();
		return -1;
	}

	//must backup build.prop first.
	backupProp();

	if(parse_area(fp_area, area, local, language, operator, reserve, customHandler)) {
		printf("=============> parse area.xml error <===========\n");
		fclose(fp_area);
		board_id_close_device();
		return -1;
	}

	fclose(fp_area);

	//get devices from ioctrl
	int i;
	char type[32];
	char dev[32];
	FILE *fp_device;

	fp_device = fopen("/cust/device.xml", "r");
	if(fp_device == NULL) {
		printf("open device.xml error!\n");
		board_id_close_device();
		return -1;
	}

	for(i = DEVICE_TYPE_TP; i < DEVICE_NUM_TYPES; i++) {
		memset(type, 0, sizeof(type));
		memset(dev, 0, sizeof(dev));
		if(board_id_get_device_name(i, type, dev)) {
			printf("===========> get device info error <===========\n");
			fclose(fp_device);
			board_id_close_device();
			return -1;
		}
		printf("get device info from ioctrl: type=%s, dev=%s \n", type, dev);
		if(parse_device(fp_device, dev, type, customHandler)) {
			printf("===========> parse device.xml error <==========\n");
			fclose(fp_device);
			board_id_close_device();
			return -1;
		}
	}

	char board_id[DEVICE_NUM_TYPES];

	memset(board_id, 0, sizeof(board_id));
	board_id_get(board_id);
	if(saveBoardIdToFile(board_id)) {
		printf("save board id to file error! \n");
		fclose(fp_device);
		board_id_close_device();
		return -1;
	}

	fclose(fp_device);
	board_id_close_device();
	return 0;
}



