#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>

#if defined (_WIN32) || defined(_WIN64)
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#else
#include<pthread.h>
#include<unistd.h>
#endif

#include "ezOtaSDK.h"
#include "ezOtaSDK_error.h"
#include "bscJSON.h"

ezOtaSDK_modules_t g_modules = {0};
ezOtaSDK_firmwarePackageList_t g_packageList = {0};
static int g_byfile = 0;
static int g_configType = 0;
static int g_need_upgrade = 0;
static int g_log_level = 3;
char g_log_path[100] = {0};
char g_dirpath[100] = {0};

#if defined (_WIN32) || defined(_WIN64)
char configInfoPath[24] = "configInfo";
#else
char configInfoPath[24] = "./configInfo";
#endif


int init_net_statet()
{
	int ret;
#if defined (_WIN32) || defined(_WIN64)
	WORD wVersionRequested;
	WSADATA wsaData;
	/* WinSock初始化 */
	wVersionRequested = MAKEWORD(2, 2);            /* 希望使用的WinSock DLL的版 */
	ret = WSAStartup(wVersionRequested, &wsaData); /* 加载套接字库 */
	if (ret != 0) {
		return -1;
	}

	
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup(); 
		return -1;
	}
#endif

	return 0;
}

static int save_devid(const char *devid, int len)
{
	int fd = open("devid", O_CREAT|O_RDWR, 0x777);
	if(fd < 0){
		printf("open devid file err:%d\n", fd);
		return -1;
	}

	int writelen = write(fd, (char *)devid, len);
	printf("write devid len:%d\n", writelen);
	close(fd);
	return 0;
}

static int get_devid(char *devid, int len)
{
	int fd = open("devid", O_RDONLY);
	if(fd < 0){
		printf("open devid file err:%d\n", fd);
		return 0;
	}

	int readlen = read(fd, devid, len);
	printf("read devid len:%d,%s\n", readlen, devid);
	close(fd);
	return readlen;
}

static int download_status(ezOtaSDK_dwStatus_e status, void *data, int len, void* userData)
{
	static int process = 0;
	

	switch(status)
	{
		case dwStatus_download_start:
			{/* 开始下载 */
				printf("start download\n");
				process = 0;
			}
			break;

		case dwStatus_download_fail:
			{/* 下载失败 */
				printf("dwStatus_download_fail error=%d\n", *(int *)data);
			}
			break;

		case dwStatus_download_suc:
			{/* 下载成功 */
				printf("dwStatus_download_suc\n");
				ezOtaSDK_firmwarePackageInfo_t *packageInfo = (ezOtaSDK_firmwarePackageInfo_t *)data;
				if(!data){
					return -1;
				}

				printf("mod_name=%s\n", packageInfo->mod_name);
				printf("fw_ver=%s\n", packageInfo->fw_ver);
				printf("digest=%s\n", packageInfo->digest);
				printf("filepath=%s\n", packageInfo->filepath);
				printf("filesize=%d\n", packageInfo->filesize);
				printf("fileType=%d\n", packageInfo->fileType);
				printf("dsc=%s\n", packageInfo->dsc);
			}
			break;
		
		case dwStatus_download_progress:/* 下载进度回调 */
			{
				if(process != *(int *)data){
					process = *(int *)data;
					printf("dwStatus_download_progress:%d\n", process);
				}
			}
			break;
		
		default:
			printf("download_status type=%d\n", status);
			break;
	}

	return 0;
}

/* 获取deviceId */
static int getDeviceId_cb(char *data, int len, void* userData)
{
	return get_devid(data, len);
}

/* 设置deviceId */
static void setDeviceId_cb(const char *data, int len, void* userData)
{
	printf("save devid:%s\n", data);
	save_devid(data, len);
}

int get_testconfig()
{
	int ret = -1;
	bscJSON* pJsRoot = NULL;
	int fd = open("config.conf", O_RDONLY);
	if(fd < 0){
		printf("open devid file err:%d\n", fd);
		return -1;
	}

	char data[1000];
	memset(data, 0, sizeof(data));
	
	do{
		int readlen = read(fd, data, sizeof(data));
		if(readlen <= 0){
			printf("read error:%d\n", readlen);
			break;
		}
		printf("read config len:%d,%s\n", readlen, data);

		pJsRoot = bscJSON_Parse(data);
		if(!pJsRoot){
			printf("pJsRoot bscJSON_Parse err\n");
			break;
		}

		bscJSON* pJsbyfile = bscJSON_GetObjectItem(pJsRoot, "byfile");
        if(pJsbyfile && (pJsbyfile->type == bscJSON_Number)){
            g_byfile = pJsbyfile->valueint;
			printf("byfile =%d\n", g_byfile);
        }
		else{
			printf("byfile invalid\n");
			break;
		}

		bscJSON* pJsconfigTypee = bscJSON_GetObjectItem(pJsRoot, "configType");
        if(pJsconfigTypee && (pJsconfigTypee->type == bscJSON_Number)){
			
            g_configType = pJsconfigTypee->valueint;
			printf("configType =%d\n", g_configType);
        }
		else{
			printf("configType invalid\n");
			break;
		}

		bscJSON* pJsmodules = bscJSON_GetObjectItem(pJsRoot, "modules");
        if(pJsmodules && (pJsmodules->type == bscJSON_Array)){
			g_modules.num = bscJSON_GetArraySize(pJsmodules);
			printf("modules num:%d\n", g_modules.num); 
			if(g_modules.num == 0){
				break;
			}
			
			unsigned int i;
			for(i=0; i<g_modules.num; i++)
			{
				g_modules.moduleList[i] = (ezOtaSDK_module_t *)malloc(sizeof(ezOtaSDK_module_t));
				if(g_modules.moduleList[i] == NULL){
					printf("moduleList malloc err\n");
					break;
				}
				memset(g_modules.moduleList[i], 0, sizeof(ezOtaSDK_module_t));

				ezOtaSDK_module_t *module = g_modules.moduleList[i];
				bscJSON* tmp = bscJSON_GetArrayItem(pJsmodules, i);
				if(!tmp || (tmp->type != bscJSON_Object)){
					printf("GetArrayItem %d err\n", i); 
					break;
				}

				bscJSON* pJssupportDiff = bscJSON_GetObjectItem(tmp, "supportDiff");
				if(pJssupportDiff && (pJssupportDiff->type == bscJSON_Number)){
					module->supportDiff = pJssupportDiff->valueint;
					printf("supportDiff = %d\n", module->supportDiff);
				}
				else{
					printf("supportDiff invalid\n");
				}

				bscJSON* pJsmod_name = bscJSON_GetObjectItem(tmp, "mod_name");
				if(pJsmod_name && (pJsmod_name->type == bscJSON_String)){
					strncpy(module->mod_name, pJsmod_name->valuestring, sizeof(module->mod_name)-1);
					printf("mod_name = %s\n", module->mod_name);
				}
				else{
					printf("mod_name invalid\n");
				}

				bscJSON* pJsfw_ver = bscJSON_GetObjectItem(tmp, "fw_ver");
				if(pJsfw_ver && (pJsfw_ver->type == bscJSON_String)){
					strncpy(module->fw_ver, pJsfw_ver->valuestring, sizeof(module->fw_ver)-1);
					printf("fw_ver = %s\n", module->fw_ver);
				}
				else{
					printf("fw_ver invalid\n");
				}
			}
        }
		else{
			printf("modules invalid\n");
			break;
		}

		bscJSON* pJsdirpath = bscJSON_GetObjectItem(pJsRoot, "dirpath");
        if(pJsdirpath && (pJsdirpath->type == bscJSON_String)){
			strncpy(g_dirpath, pJsdirpath->valuestring, sizeof(g_dirpath)-1);
			printf("dirpath = %s\n", g_dirpath);
        }
		else{
			printf("dirpath invalid\n");
		}

		bscJSON* pJslog_path = bscJSON_GetObjectItem(pJsRoot, "log_path");
        if(pJslog_path && (pJslog_path->type == bscJSON_String)){
			strncpy(g_log_path, pJslog_path->valuestring, sizeof(g_log_path)-1);
			printf("log_path = %s\n", g_log_path);
		}

		bscJSON* pJslog_level = bscJSON_GetObjectItem(pJsRoot, "log_level");
        if(pJslog_level && (pJslog_level->type == bscJSON_Number)){
			g_log_level = pJslog_level->valueint;
			printf("log_level = %d\n", g_log_level);
		}

		ret = 0;
	}while(0);
	
	if(fd > 0){
		close(fd);
	}

	if(pJsRoot){
		bscJSON_Delete(pJsRoot);
	}
	return ret;
}

int init_configinfo(ezOtaSDK_config_info_t *config)
{
	int ret = -1;
	bscJSON* pJsRoot = NULL;
	int fd = open(configInfoPath, O_RDONLY);
	if(fd < 0){
		printf("open devid file err:%d\n", fd);
		return -1;
	}

	char data[1000];
	memset(data, 0, sizeof(data));
	
	do{
		int readlen = read(fd, data, sizeof(data));
		if(readlen <= 0){
			printf("read error:%d\n", readlen);
			break;
		}
		printf("read config len:%d,%s\n", readlen, data);

		pJsRoot = bscJSON_Parse(data);
		if(!pJsRoot){
			printf("pJsRoot bscJSON_Parse err\n");
			break;
		}

		bscJSON* pJsdasHttpPort = bscJSON_GetObjectItem(pJsRoot, "dasHttpPort");
        if(pJsdasHttpPort && (pJsdasHttpPort->type == bscJSON_Number)){
            config->dasHttpPort = pJsdasHttpPort->valueint;
			printf("dasHttpPort = %d\n", config->dasHttpPort);
        }
		else{
			printf("dasHttpPort invalid\n");
			break;
		}

		bscJSON* pJsdasHttpHost = bscJSON_GetObjectItem(pJsRoot, "dasHttpHost");
        if(pJsdasHttpHost && (pJsdasHttpHost->type == bscJSON_String)){
			strncpy(config->dasHttpHost, pJsdasHttpHost->valuestring, sizeof(config->dasHttpHost)-1);
			printf("dasHttpHost = %s\n", config->dasHttpHost);
        }
		else{
			printf("dasHttpHost invalid\n");
			break;
		}

		bscJSON* pJsdeviceMac = bscJSON_GetObjectItem(pJsRoot, "deviceMac");
        if(pJsdeviceMac && (pJsdeviceMac->type == bscJSON_String)){
			strncpy(config->deviceMac, pJsdeviceMac->valuestring, sizeof(config->deviceMac)-1);
			printf("deviceMac = %s\n", config->deviceMac);
        }
		else{
			printf("deviceMac invalid\n");
			break;
		}

		bscJSON* pJsdevVersion = bscJSON_GetObjectItem(pJsRoot, "devVersion");
        if(pJsdevVersion && (pJsdevVersion->type == bscJSON_String)){
			strncpy(config->devVersion, pJsdevVersion->valuestring, sizeof(config->devVersion)-1);
			printf("devVersion = %s\n", config->devVersion);
        }
		else{
			printf("devVersion invalid\n");
			break;
		}

		bscJSON* pJsproductLicense = bscJSON_GetObjectItem(pJsRoot, "productLicense");
        if(pJsproductLicense && (pJsproductLicense->type == bscJSON_Object)){
			bscJSON* pJsproductKey = bscJSON_GetObjectItem(pJsproductLicense, "productKey");
			if(pJsproductKey && (pJsproductKey->type == bscJSON_String)){
				strncpy(config->productLicense.productKey, pJsproductKey->valuestring, sizeof(config->productLicense.productKey)-1);
				printf("productKey = %s\n", config->productLicense.productKey);
			}
			else{
				printf("productKey invalid\n");
				break;
			}

			bscJSON* pJsdeviceName = bscJSON_GetObjectItem(pJsproductLicense, "deviceName");
			if(pJsdeviceName && (pJsdeviceName->type == bscJSON_String)){
				strncpy(config->productLicense.deviceName, pJsdeviceName->valuestring, sizeof(config->productLicense.deviceName)-1);
				printf("deviceName = %s\n", config->productLicense.deviceName);
			}
			else{
				printf("deviceName invalid\n");
				break;
			}

			bscJSON* pJsdeviceLicense = bscJSON_GetObjectItem(pJsproductLicense, "deviceLicense");
			if(pJsdeviceLicense && (pJsdeviceLicense->type == bscJSON_String)){
				strncpy(config->productLicense.deviceLicense, pJsdeviceLicense->valuestring, sizeof(config->productLicense.deviceLicense)-1);
				printf("deviceLicense = %s\n", config->productLicense.deviceLicense);
			}
			else{
				printf("deviceLicense invalid\n");
				break;
			}
        }
		else{
			printf("productLicense invalid\n");
			break;
		}

		ret = 0;
	}while(0);
	
	if(fd > 0){
		close(fd);
	}

	if(pJsRoot){
		bscJSON_Delete(pJsRoot);
	}
	return ret;
}



static void upgrade_task(ezOtaSDK_firmwarePackageInfo_t *info)
{
	if (info == NULL) {
		printf("args NULL\n");
		return;
	}
	ezOtaSDK_download_info_t dwInfo = {0};
	dwInfo.byfile = g_byfile;
	dwInfo.dwStatus_cb = download_status;
	dwInfo.fileType = info->fileType;
	strncpy(dwInfo.fw_ver, info->fw_ver, sizeof(dwInfo.fw_ver)-1);
	strncpy(dwInfo.fw_ver_dst, info->fw_ver_dst, sizeof(dwInfo.fw_ver_dst)-1);
	strncpy(dwInfo.mod_name, info->mod_name, sizeof(dwInfo.mod_name)-1);
	dwInfo.dirpath = g_dirpath;
	int ret = ezOtaSDK_start_download(&dwInfo);
	if (ret != 0) {
		printf("ezOtaSDK_start_download err:%d\n", ret);
	}
	else {
		printf("ezOtaSDK_start_download ok\n");
	}

	return;
}

static void *upgrade_task_linux(void *args)
{
	ezOtaSDK_firmwarePackageInfo_t *info = (ezOtaSDK_firmwarePackageInfo_t *)args;
	upgrade_task(info);
	return NULL;
}

static unsigned int _stdcall upgrade_task_win(void *aArg)
{
	ezOtaSDK_firmwarePackageInfo_t *info = (ezOtaSDK_firmwarePackageInfo_t *)aArg;
	upgrade_task(info);
	return 0;
}

void prinHelpInfo()
{
	printf("input \"q\" exit\n");
	printf("input \"query\" query package\n");
	printf("input \"dw\" start download package\n");
	printf("input \"stop\" stop doanload package\n");
	printf("input \"reportok\" report upgrade suc\n");
	printf("input \"reportfail\" report upgrade fail\n");
	printf("please input cmd:\n");
}

int selectModule()
{
	char message[100] = { 0 };

	printf("select module\n");
	for (int i = 0; i < g_packageList.num; i++)
	{
		printf("input %d select %s\n", i+1, g_packageList.PackageInfo[i]->mod_name);
	}

	memset(message, 0, 100);
	fgets(message, 100, stdin);

	int num = atoi(message);
	if (num > g_packageList.num || num == 0) {
		printf("select module number error[%d]>[%d], set num=1\n", num, g_packageList.num);
		num = 0;
	}
	else{
		printf("select module %d\n", num);
		num--;
	}
	
	return num;
}

int main()
{
	int ret;
	int num = 0;
	
	ret = get_testconfig();
	if(ret){
		return -1;
	}
	init_net_statet();

	{/* 设置SDK日志级别 */
		if(strlen(g_log_path)){
			ezOtaSDK_set_log_level(g_log_level, g_log_path);
		}
		else{
			ezOtaSDK_set_log_level(g_log_level, NULL);
		}
	}
	
	/* SDK初始化 */
	ezOtaSDK_init_info_t init_info = {0};
	{
		ezOtaSDK_config_info_t config = {0};
		init_info.byFile = g_configType;
		
		if(g_configType){
			init_info.configInfo = (void *)configInfoPath;
		}
		else{
			ret = init_configinfo(&config);
			if(ret){
				return -1;
			}
			init_info.configInfo = (void *)&config;
		}
		
		init_info.getDevid_cb = getDeviceId_cb;
		init_info.setDevid_cb = setDeviceId_cb;

		ret = ezOtaSDK_init(&init_info);
		if(ret){
			printf("ezOtaSDK_init error:0x%x\n", ret);
			return -1;
		}

		printf("ezOtaSDK_init ok\n");
	}
	
	char message[100] ={0};

	while (1)
	{
		prinHelpInfo();
		memset(message, 0, 100);
		fgets(message, 100, stdin);

		if (!strcmp(message, "q\n"))
		{/* 退出 */
			break;
		}

		if (!strcmp(message, "query\n"))
		{/* 查包 */
			ret = ezOtaSDK_query_package(&g_modules, &g_packageList);
			if (ret == ota_errcode_no_package) {
				printf("ezOtaSDK_query_package no new package\n");
			}
			else if (ret != 0) {
				printf("ezOtaSDK_query_package err:%d\n", ret);
			}
			else {
				printf("ezOtaSDK_query_package ok\n");
				printf("g_packageList num=%d\n", g_packageList.num);
				for(int i=0; i<g_packageList.num; i++)
				{
					printf("mod_name:%s\n", g_packageList.PackageInfo[i]->mod_name);
					printf("fw_ver:%s\n", g_packageList.PackageInfo[i]->fw_ver);
					printf("dsc:%s\n", g_packageList.PackageInfo[i]->dsc);
					printf("digest:%s\n", g_packageList.PackageInfo[i]->digest);
					printf("filesize:%d\n", g_packageList.PackageInfo[i]->filesize);
					printf("fileType:%d\n", g_packageList.PackageInfo[i]->fileType);
					printf("battery_limit:%d\n", g_packageList.PackageInfo[i]->conditions.battery_limit);
					printf("network_limit:%d\n", g_packageList.PackageInfo[i]->conditions.network_limit);
					printf("silent_upgrade:%d\n", g_packageList.PackageInfo[i]->conditions.silent_upgrade);
					printf("startTime:%s\n", g_packageList.PackageInfo[i]->conditions.startTime);
					printf("endTime:%s\n", g_packageList.PackageInfo[i]->conditions.endTime);
					printf("\n#####################################\n\n");
				}
			}
		}

		if (!strcmp(message, "dw\n"))
		{/* 开始下载 */
			if(g_packageList.num == 0){
				printf("not need upgrade\n");
				continue;
			}
			num = selectModule();
#if defined (_WIN32) || defined(_WIN64)
			HANDLE hd;
			unsigned int threadID = 0;

			hd = (HANDLE)_beginthreadex(NULL, 0, upgrade_task_win, (void *)g_packageList.PackageInfo[num], 0, &threadID);
			if (NULL == hd) {
				printf("upgrade_task create error\n");
				break;
			}
#else
			pthread_t p;
			ret = pthread_create(&p, NULL, upgrade_task_linux, (void *)g_packageList.PackageInfo[num]);
			if (ret) {
				printf("upgrade_task create error\n");
				break;
			}
			pthread_detach(p);
#endif
		}

		if (!strcmp(message, "stop\n"))
		{/* 停止下载 */
			num = selectModule();
			ret = ezOtaSDK_stop_download(g_modules.moduleList[num]->mod_name);
			if (ret != 0) {
				printf("ezOtaSDK_stop_download err:%d\n", ret);
			}
			else {
				printf("ezOtaSDK_stop_download ok\n");
			}
		}

		if (!strcmp(message, "reportok\n"))
		{/* 上报升级成功 */
			num = selectModule();
			ret = ezOtaSDK_report_upgrade_result(g_modules.moduleList[num], 0);
			if (ret != 0) {
				printf("ezOtaSDK_report_upgrade_result err:%d\n", ret);
			}
			else {
				printf("ezOtaSDK_report_upgrade_result ok\n");
			}
		}

		if (!strcmp(message, "reportfail\n"))
		{/* 上报升级失败 */
			num = selectModule();
			ret = ezOtaSDK_report_upgrade_result(g_modules.moduleList[num], upgrade_fail);
			if (ret != 0) {
				printf("ezOtaSDK_report_upgrade_result err:%d\n", ret);
			}
			else {
				printf("ezOtaSDK_report_upgrade_result ok\n");
			}
		}
	}

	ezOtaSDK_fini();

	for(int i=0; i<MAX_MODULE_NUM; i++)
	{
		if(g_modules.moduleList[i] != NULL){
			free(g_modules.moduleList[i]);
			g_modules.moduleList[i] = NULL;
		}
	}

	g_modules.num = 0;

	ezOtaSDK_log_stop();
	return 0;
}
