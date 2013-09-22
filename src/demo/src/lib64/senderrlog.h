#ifndef _SEND_ERR_LOG_H_
#define _SEND_ERR_LOG_H_

/************************************************************
描述: 收集各平台重要数据接口
作者: ronniechen
创建日期: 2008-12-10
修订日期:  次数     修改人   修改内容
            1       ronniechen  创建
************************************************************/
#include <vector>
#include <string>
using namespace std;

#define SEND_LOG_EMERG(serverip, module, errcode, msg) do \
{ \
itil_err_log::sendError(serverip, module,"高", errcode, msg, __FILE__, __LINE__, __FUNCTION__);\
} while (0);

#define SEND_LOG_ERROR(serverip, module, errcode, msg) do \
{ \
itil_err_log::sendError(serverip, module,"中", errcode, msg, __FILE__, __LINE__, __FUNCTION__);\
} while (0);

#define SEND_LOG_WARN(serverip, module, errcode, msg) do \
{ \
itil_err_log::sendError(serverip, module,"低", errcode, msg, __FILE__, __LINE__, __FUNCTION__);\
} while (0);

#define SEND_LOG_EMERG2(serverip, module, srcfile, srcline, func, errcode, msg) do \
{ \
	itil_err_log::sendError(serverip, module,"高", errcode, msg, srcfile, srcline, func);\
} while (0);

namespace itil_err_log
{
        int sendError(const string& serverip, const string& module, const string& level, 
		int errcode, const string& msg, const string& srcfile,  int srcline, const string& func);
	/*
        this function is used to send a msg to server by udp.

        params:
		serverip: itil日志服务器IP
		module:  比如发布系统的某一个模块名称, 层级用.隔开, 如 "发布系统.章鱼分发.***"
		level:    err level, should be one of( "高" "中" "低" )
		errcode:  errcode
		msg:    user error msg
		srcfile: always __FILE__
		srcline: always __LINE__
		func: always __FUNCTION__
	
                note:all sending data is limited 4096 bytes.
        return values:
                0: succeed.
                -1: connect server failed.
                -2: send data failed.
                -3: received data failed.
                -4: unpack return data error.
        */
}//end namespace
#endif

