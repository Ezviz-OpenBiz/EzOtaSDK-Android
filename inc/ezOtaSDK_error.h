/**
 * \file      ezOtaSDK_error.h  
 * 
 * \copyright © 2017-2021 Ezviz Inc.
 * 
 * \brief     
 * 
 * \author    zoujinwei
 * 
 * \date      2022/04/18
 */

#ifndef H_EZOTASDK_ERROR_H_
#define H_EZOTASDK_ERROR_H_



    /**
    * \brief   SDK接口错误码信息 
    */
	typedef enum
    {
        ota_errcode_suc = 0,				    ///< 【0】,没有错误 
        ota_errcode_param_invalid,              ///< 【1】,参数无效 
        ota_errcode_dirpath_invalid,            ///< 【2】,文件目录无效，无法打开 
        ota_errcode_callback_null,              ///< 【3】,回调函数未注册 
        ota_errcode_task_busy,				    ///< 【4】,任务繁忙，不可以并行调用该接口  
        ota_errcode_request_busy,               ///< 【5】,请求频繁，距离上次请求未过30s   
        ota_errcode_sign,				        ///< 【6】,签名校验失败 
        ota_errcode_mem,				        ///< 【7】,内存申请失败  
        ota_errcode_getrandom_fail,             ///< 【8】,获取随机数失败 
        ota_errcode_getcipher_fail,             ///< 【9】,获取服务端cipher失败 
        ota_errcode_checkcipher_fail,           ///< 【10】,校验服务端cipher失败  
        ota_errcode_checkdevid_fail,            ///< 【11】,校验devid不合法   
        ota_errcode_report_modules_err,         ///< 【12】,上报模块信息失败 
        ota_errcode_query_modules_err,          ///< 【13】,查包失败 
        ota_errcode_no_package,                 ///< 【14】,没有查到最新的包，确认是否有部署更高版本的固件包 
        ota_errcode_http_download_err,          ///< 【15】,https下载包失败 
        ota_errcode_open_file_err,              ///< 【16】,打开文件失败 
        ota_errcode_read_file_err,              ///< 【17】,读文件失败 
        ota_errcode_wirte_file_err,             ///< 【18】,写文件失败 
        ota_errcode_read_msg_err,               ///< 【19】,接收到的消息内容错误，消息格式不对 
        ota_errcode_aes_enc_err,                ///< 【20】,aes加密失败 
        ota_errcode_aes_dec_err,                ///< 【21】,aes解密失败 
        ota_errcode_file_sign_err,			    ///< 【22】,文件签名校验失败 
        ota_errcode_url_err,                    ///< 【23】,url内容错误 
        ota_errcode_stop_download,              ///< 【24】,停止下载
        ota_errcode_alread_stop_download,       ///< 【25】,已经停止下载/未开始下载/已完成下载
        ota_errcode_download_not_found,         ///< 【26】,查找不到下载列表  
        ota_errcode_downloading,                ///< 【27】,正在下载，异常调用   
        ota_errcode_alread_init,                ///< 【28】,SDK已经初始化 
        ota_errcode_not_init,                   ///< 【29】,SDK未初始化 
        ota_errcode_json_parse_err,             ///< 【30】,json解析错误 
        ota_errcode_getdevid_err,               ///< 【31】,向设备获取devid失败 

        /* 设备网络相关错误码 */
        ota_errcode_dns = 100,				    ///< 【100】,域名解析失败，抓包定位或检查dns服务器配置问题 
        ota_errcode_socket_connect_err,         ///< 【101】,socket连接失败 
        ota_errcode_socket_read_err,            ///< 【102】,socket接收数据失败，网络问题，抓包定位 
        ota_errcode_socket_write_err,           ///< 【103】,socket发送数据失败，网络问题，抓包定位 
        ota_errcode_socket_connect_timeout,     ///< 【104】,socket连接超时，网络问题，可抓包定位 
        ota_errcode_socket_read_timeout,        ///< 【105】,socket发送超时，网络问题，可抓包定位 
        ota_errcode_socket_write_timeout,       ///< 【106】,socket接收超时，网络问题，可抓包定位 
        ota_errcode_http_ack_err,               ///< 【107】,http响应状态错误 
        ota_errcode_socket_other_err,           ///< 【108】,socket其他错误 

        /* 服务端响应错误码 */
        ota_errcode_license_invalid = 1001,     ///< 【1001】,license未生效 
        ota_errcode_random_code_invalid,        ///< 【1002】,随机数失效 
        ota_errcode_random_code_err,            ///< 【1003】,随机数错误 
        ota_errcode_device_cipher_err,          ///< 【1004】,设备cipher错误 
        ota_errcode_product_err,                ///< 【1005】,产品不存在 
        ota_errcode_deviceid_err,               ///< 【1006】,device id错误，需要重新申请(一般是该设备还未申请过devid) 
        ota_errcode_license_err,                ///< 【1007】,license不存在 
        ota_errcode_device_activated,           ///< 【1008】,设备已激活，请勿重复激活 
        ota_errcode_license_active_err,         ///< 【1009】,该设备激活失败,请稍后重试   
        ota_errcode_license_not_active,         ///< 【1010】,license未激活，该设备未激活，请先激活 
        ota_errcode_license_activate_ceiling,   ///< 【1011】,license激活设备数量已达上限 
        ota_errcode_license_illegal_device,     ///< 【1012】,非法设备，请添加到设备白名单列表 
        ota_errcode_deviceid_err2,              ///< 【1013】,device id错误，不允许申请(一般是该设备已经申请过devid) 
        ota_errcode_encrypt_err=3003,           ///< 【3003】,设备加密报文错误 
    }ezOtaSDK_errcode_e;


    /**
    * \brief   OTA升级失败错误码信息 
    */
	typedef enum
    {
        upgrade_suc                 = 0,                        ///< 升级成功 
        upgrade_filelen_err         = 0x00500032,				///< 文件大小和查询的信息不匹配 
        upgrade_filelen_overflow    = 0x00500033,				///< 文件大小超过分区大小 
        upgrade_dns_err             = 0x00500034,				///< 下载地址域名解析失败 
        upgrade_socket_timeout      = 0x00500035,				///< 网络不稳定，下载超时（超过最大重试次数） 
        upgrade_md5_err             = 0x00500036,				///< 摘要值不匹配 
        upgrade_md5_fail            = 0x00500037,				///< 签名校验失败 
        upgrade_package_invalid     = 0x00500038,				///< 升级包损坏（内部格式不对） 
        upgrade_mem_overflow        = 0x00500039,				///< 内存不足 
        upgrade_brun_err            = 0x0050003a,				///< 烧录固件出错 
        upgrade_fun_err             = 0x0050003b,				///< 底层接口出现未知错误 
        upgrade_stop                = 0x0050003c,				///< 用户主动停止 
        upgrade_fail                = 0x0050003d,				///< 升级失败，通过备份系统或最小系统恢复 
        upgrade_no_package          = 0x0050003e,				///< 平台没有可用的升级包版本（设备主动查询升级包返回值） 
    }ezOtaSDK_upgrade_errcode_e;



#endif//H_EZOTASDK_ERROR_H_
