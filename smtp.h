/*
SMTP框架(Linux版本)
作者：SpongeBob
联系方式:2443425920@qq.com
创建时间:2023/6/2
备注:Base64不由本人编码，借用于作者Ahmed Elzoughby的代码

***********************************************************
* Base64 library                                          *
* @author Ahmed Elzoughby                                 *
* @date July 23, 2017                                     *
* Purpose: encode and decode base64 format                *
***********************************************************
*/

#ifndef SMTP_H
#define SMTP_H

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

/***********************************************
Encodes ASCCI string into base64 format string
@param plain ASCII string to be encoded
@return encoded base64 format string
***********************************************/
// base64编码
char* base64_encode(const char* plain);
/***********************************************
decodes base64 format string into ASCCI string
@param plain encoded base64 format string
@return ASCII string to be encoded
***********************************************/
// base64解码
char* base64_decode(const char* cipher);

/*---------------------SMTP发邮件框架--------------------------*/
class bSMTP
{
private:
    int  m_rec;        // 用于记录执行步骤的位置，防止跳步骤操作
    int  m_socket;     // 通信sock
    char m_from[51];   // 登录邮箱

    // 发送报文HELO MSG确认连接
    // 返回值:0-成功 -1-失败
    int sendHelloMsg();

    // 发送报文AUTH LOGIN登录请求
    // 返回值:0-成功 -1-失败
    int sendAuthLogin();

    // 发送报文DATA
    // 返回值:0-成功 -1-失败
    int sendData();

    // 发送报文Quit
    // 返回值:0-成功 -1-失败
    int sendEnd();

    // 关闭资源
    void Close();

public:
    // 构造函数
    bSMTP();

    // 连接邮箱服务器
    // ipaddr:邮箱域名  port:端口号
    // 返回值:0-成功 -1-失败
    int connectServer(const char* ipaddr,int port);

    // 发送报文用户名和识别码
    // UserName:用户名 Password:识别码
    // 返回值:0-成功 -1-失败
    int sendAuthInfo(const char* UserName,const char* PassWord);

    // 发送报文收件人邮箱
    // to:收件人邮箱
    // 返回值:0-成功 -1-失败
    int sendRecipient(const char* to);

    // 发送报文邮件主题和内容
    // subject:主题 text:内容
    // 返回值:0-成功 -1-失败
    int sendMail(const char *subject,const char *text);

    // 析构函数
    ~bSMTP();
};

#endif
