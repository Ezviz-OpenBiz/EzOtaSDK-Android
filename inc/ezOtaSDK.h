/**
 * \file      ezOtaSDK.h  
 * 
 * \copyright © 2017-2021 Ezviz Inc.
 * 
 * \brief     升级模块对外接口
 * 
 * \author    zoujinwei
 * 
 * \version   V2.0.1
 * 
 * \date      2022/09/20
 */

#ifndef H_EZOTASDK_H_
#define H_EZOTASDK_H_

#if (defined(_WIN32) || defined(_WIN64))
#if defined(LIBOTA_EXPORTS)
#define OTA_API __declspec(dllexport)
#else
#define OTA_API __declspec(dllimport)
#endif
#else
#define OTA_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    #define DEVICE_ID_LEN                32     ///< devid长度 
    #define MAX_MODULE_NUM              64      ///< 最多允许查询64个模块 

    typedef enum{
        log_level_error = 1,        ///< 某个业务出错 
        log_level_wran,             ///< 打印业务过程中必要的关键信息 
        log_level_info,             ///< 较详细的信息 
        log_level_debug,            ///< 更为详细的信息 
        log_level_verbose           ///< 不限制打印 
    }ezOtaSDK_logLevel_e;

    /**
    * \brief   固件包下载状态 
    */
    typedef enum
    {
        dwStatus_download_start = 0,        ///< 开始下载，data 为NULL  
        dwStatus_download_fail,             ///< 下载失败，data 对应 int*错误码信息,ref/ezOtaSDK_errcode_e
        dwStatus_download_suc,             ///< 下载成功，data 对应 ezOtaSDK_firmwarePackageInfo_t 
        dwStatus_download_progress,        ///< 下载进度，data 对应进度百分比 0-100 
    }ezOtaSDK_dwStatus_e;

    /**
    * \brief   固件包下载/升级条件 
    */
    typedef struct
    {
        int   silent_upgrade;               ///< 升级方式：0-触发升级,1-静默升级 
        int   battery_limit;                ///< 升级时和下载固件包时最低电量要求：限制百分比:0-100,0表示不限制 
        int   network_limit;                ///< 固件包下载网络要求 0-不限制,1-非流量模式可以下载（非4G/5G模式可下载） 
        char  startTime[12];                ///< 允许/建议下载和升级的开始时间00:00（小时：分钟） 
        char  endTime[12];                 ///< 允许/建议下载和升级的结束时间 00:00（小时：分钟） 
    }ezOtaSDK_upgrade_conditions_t;


    /**
    * \brief   查询到的固件包信息  
    */
    typedef struct
    {
        int   fileType;                     ///< 固件包类型，0：整包，1：差分包 
        unsigned int   filesize;            ///< 升级包文件大小 
        char  mod_name[256];                ///< 模块名称/型号 
        char  fw_ver[64];                   ///< 模块最新的固件版本号, 格式为:V1.1.0 build 220427格式 
        char  fw_ver_dst[64]; 	            ///< 如果为差分包，该参数为当前固件版本
        char  *filepath;                    ///< 升级包文件保存路径，如果byfile=0则该路径无效  
        char  digest[33];                   ///< 升级包摘要,MD5小写摘要 
        char  dsc[512];                     ///< 升级包描述 
        ezOtaSDK_upgrade_conditions_t conditions;   ///< 升级条件 
    }ezOtaSDK_firmwarePackageInfo_t;

    /**
    * \brief   查询到的固件包列表  
    */
    typedef struct
    {
        int   num;                     
        ezOtaSDK_firmwarePackageInfo_t *PackageInfo[MAX_MODULE_NUM];
    }ezOtaSDK_firmwarePackageList_t;

    /** 
     * \brief		升级包下载回调
     * \method		ezOtaSDK_dwFile_cb
     * \param   fileInfo:文件信息， ezOtaSDK_firmwarePackageInfo_t
     * \param   totalLen：文件总大小
     * \param   offset：文件下载偏移
     * \param   data：文件流
     * \param   len：文件流长度
     * \param   userData：用户指针
     * \return 	成功返回0 失败返回非0
     */
    typedef void (*ezOtaSDK_dwFile_cb)(void *fileInfo, unsigned int totalLen, unsigned int offset, char *data, unsigned int len, void* userData);
	
    /** 
     * \brief		升级包下载状态回调函数
     * \method		ezOtaSDK_dwStatus_cb
     * \return 	成功返回0 失败返回非0
     */
    typedef int (*ezOtaSDK_dwStatus_cb)(ezOtaSDK_dwStatus_e status, void *data, int len, void* userData);

    /** 
     * \brief		向设备查询devid
     * \method		ezOtaSDK_getDeviceId_cb
     * \return 	成功返回devid长度 失败返回-1
     */
    typedef int (*ezOtaSDK_getDeviceId_cb)(char *data, int len, void* userData);

    /** 
     * \brief		通知设备保存并固话devid
     * \method		ezOtaSDK_setDeviceId_cb
     */
    typedef void (*ezOtaSDK_setDeviceId_cb)(const char *data, int len, void* userData);

    /**
    * \brief   设备产品license信息 
    */
    typedef struct{
        char productKey[64];        ///<[in] 必须：通过license申请接口申请出来：productKey/PID
        char deviceName[64];        ///<[in] 必须：设备自定义的名称，每台设备必须保证不一致，可用设备自定义序列号、MAC、SN等  
        char deviceLicense[64];     ///<[in] 必须：通过license申请接口申请出来：deviceLicense
    }ezOtaSDK_license_info_t;

    /**
    * \brief   设备升级模块信息 
    */
    typedef struct
    {
        int  supportDiff;       ///<[in] 必须：0:不支持差分，1:支持差分 
        char mod_name[256];     ///<[in] 必须：模块名称/设备型号/固件识别码 
        char fw_ver[24];        ///<[in] 必须：模块固件版本号, 格式必须为V1.0.0 build 220427格式 
    }ezOtaSDK_module_t;

    /**
    * \brief   设备升级模块信息列表
    */
    typedef struct
    {
        unsigned int num;                   ///<[in] 必须：升级模块数量，最多支持查询64个模块
        ezOtaSDK_module_t *moduleList[MAX_MODULE_NUM];  ///<[in] 必须：升级模块信息列表,moduleList[0]为设备自己的版本信息 
    } ezOtaSDK_modules_t;

    typedef struct{
		int byfile;                             ///<[in] 必须：OTA文件临时存储方式，1：SDK自己写文件，0：SDK回调给设备保存 
        int fileType;                           ///<[in] 必须：固件类型，0：整包，1：差分包 
        char mod_name[256];                     ///<[in] 必须：模块名称 
        char fw_ver[64];                        ///<[in] 必须：需要下载的固件版本号, 格式为:V1.1.0 build 220427格式 
        char fw_ver_dst[64]; 	                ///<[in] 非必须：如果为差分包需要输入当前模块的固件版本号
        ezOtaSDK_dwFile_cb dwFile_cb;           ///<[in] 非必须：byfile=0时必须实现 
        ezOtaSDK_dwStatus_cb dwStatus_cb;       ///<[in] 非必须：固件包下载状态回调函数 
        char *dirpath;							///<[in] 非必须：byfile=1时必须设置，ota文件临时存放目录路径，目录后面不要带/ 
        void *userData;                         ///<[in] 非必须：用户指针 
	}ezOtaSDK_download_info_t;

    /**
    * \brief   设备基础信息，不可丢失
    */
    typedef struct{        
        unsigned short dasHttpPort;             ///<[in] 必须：das-http端口 
        char dasHttpHost[64];                   ///<[in] 必须：das-http域名 
        char deviceMac[18];                     ///<[in] 必须：设备MAC地址 
        char devVersion[64];                    ///<[in] 必须：设备固件版本 
		ezOtaSDK_license_info_t productLicense; ///<[in] 必须：产品license 
	}ezOtaSDK_config_info_t;

    typedef struct{
        int byFile;                             ///<[in] 必须：0-configInfo为ezOtaSDK_config_info_t结构体,1-configInfo为文件路径 
        ezOtaSDK_getDeviceId_cb getDevid_cb;    ///<[in] 必须：
        ezOtaSDK_setDeviceId_cb setDevid_cb;    ///<[in] 必须：
        void *configInfo;						///<[in] 必须：如果byFile=1，configInfo为文件路径，文件内容为ezOtaSDK_config_info_t对应的json内容 
        void *userData;                         ///<[in] 非必须，用户指针,默认为NULL 
	}ezOtaSDK_init_info_t;


    /** 
     *  \brief      SDK初始化 
     *  \method     ezOtaSDK_init
     *  \param[in] 	info:初始化信息
     *  \return     0-成功,other-失败\ref ezOtaSDK_errcode_e
     */
	OTA_API int ezOtaSDK_init(ezOtaSDK_init_info_t *info);

    /** 
     *  \brief      SDK反初始化 
     *  \method     ezOtaSDK_fini
     *  \return     0-成功,other-失败\ref ezOtaSDK_errcode_e
     */
	OTA_API int ezOtaSDK_fini();

    /** 
     *  \brief      查询升级接口    
     *  \method     ezOtaSDK_query_package
     *  \param[in] 	modules:模块信息
     *  \return     成功返回0，失败返回非0
     *  \retval     0 - 成功
     *  \retval     ota_errcode_no_package-没有查到有部署固件包
     *  \retval     非0 - 失败，\ref ezOtaSDK_errcode_e
     */
	OTA_API int ezOtaSDK_query_package(ezOtaSDK_modules_t *modules, ezOtaSDK_firmwarePackageList_t *packageList);

    /** 
     *  \brief      开始下载固件包
     *  \method     ezOtaSDK_start_download
     *  \param[in] 	dwInfo:需要下载的固件包信息
     *  \return     0-成功,other-失败\ref ezOtaSDK_errcode_e
     */
	OTA_API int ezOtaSDK_start_download(ezOtaSDK_download_info_t *dwInfo);


    /** 
     *  \brief      停止下载固件包
     *  \method     ezOtaSDK_stop_download
     *  \param[in] 	mod_name 模块名称/模块型号/固件识别码
     *  \return     0-成功,other-失败\ref ezOtaSDK_errcode_e
     */
    OTA_API int ezOtaSDK_stop_download(const char* mod_name);

    /** 
     *  \brief    上报升级结果
     *  \method     ezOtaSDK_report_upgrade_result
     *  \param[in] 	errCode,0-升级成功，其他-升级失败错误码，失败错误码使用ezDevSDK_error.h文件内的ezOtaSDK_upgrade_errcode_e
     *  \return     0-成功,other-失败\ref ezOtaSDK_errcode_e
     */
	OTA_API int ezOtaSDK_report_upgrade_result(ezOtaSDK_module_t *module, int errCode);


    /** 
     *  \brief    初始化日志接口并设置日志级别，在SDK初始化前设置
     *  \method     ezOtaSDK_set_log_level
     *  \param[in] 	log_level 日志级别ezOtaSDK_logLevel_e
     *  \param[in]  log_path 非必须，日志文件保存路径，默认在可执行文件目录下
     *  \return     0-成功,other-失败\ref ezOtaSDK_errcode_e
     */
    OTA_API int ezOtaSDK_set_log_level(int log_level, const char *log_path);

	/**
	*  \brief    关闭日志输出，在SDK反初始化后调用，下次需要重新调用ezOtaSDK_set_log_level开启日志 
	*  \method     ezOtaSDK_log_stop
	*  \return     0-成功,other-失败\ref ezOtaSDK_errcode_e
	*/
	OTA_API int ezOtaSDK_log_stop();
#ifdef __cplusplus
}
#endif

#endif//H_EZOTASDK_H_
