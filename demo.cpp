// 演示程序

#include "smtp.h"

int main(int agrc,char *argv[])
{
    bSMTP smtp;

    smtp.connectServer("邮箱域名(smtp.qq.com)",587);

    smtp.sendAuthInfo("用户名(***@qq.com)","识别码(一串乱码英文)");

    smtp.sendRecipient("收件人邮箱(***@qq.com)");

    smtp.sendMail("主题(Subject)","内容(Text)");

    return 0;
}
