# 编译demo
```txt
cmake ./
make
```

# 文件功能
config.conf---配置文件，可修改
testota.cpp----demo应用程序，可修改

# config配置说明
```c
{
	"byfile":1,			//OTA文件临时存储方式，1：SDK自己保存文件，0：SDK回调给设备保存 
	"configType":1,		
	"modules":[	//模块信息，支持多模块
		{
			"supportDiff":0,	//1：允许差分升级，如果有查分包则下载查分包，没有差分包则下载整包，0只允许整包升级
			"mod_name":"ZJW_TEST_SWITCH",	//设备型号或模块名称
			"fw_ver":"V1.0.0 build 211115"	//设备版本号
		}
	],
	"dirpath":"file",	//OTA固件包存储路径 
	"log_level":4,		//日志级别 
	"log_path":"testlog.log"	//日志文件保存路径 
}
```



