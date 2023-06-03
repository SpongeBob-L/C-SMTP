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

#include "smtp.h"

/*---------------------Base64编码--------------------------*/

char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
// base64编码
char* base64_encode(const char* plain) {

    char counts = 0;
    char buffer[3];
    char* cipher = (char *) malloc(strlen(plain) * 4 / 3 + 4);
    int i = 0, c = 0;

    for(i = 0; plain[i] != '\0'; i++) {
        buffer[counts++] = plain[i];
        if(counts == 3) {
            cipher[c++] = base46_map[buffer[0] >> 2];
            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            cipher[c++] = base46_map[((buffer[1] & 0x0f) << 2) + (buffer[2] >> 6)];
            cipher[c++] = base46_map[buffer[2] & 0x3f];
            counts = 0;
        }
    }

    if(counts > 0) {
        cipher[c++] = base46_map[buffer[0] >> 2];
        if(counts == 1) {
            cipher[c++] = base46_map[(buffer[0] & 0x03) << 4];
            cipher[c++] = '=';
        } else {                      // if counts == 2
            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            cipher[c++] = base46_map[(buffer[1] & 0x0f) << 2];
        }
        cipher[c++] = '=';
    }

    cipher[c] = '\0';   /* string padding character */
    return cipher;
}

// base64解码
char* base64_decode(const char* cipher) {

    char counts = 0;
    char buffer[4];
    char* plain = (char *) malloc(strlen(cipher) * 3 / 4);
    int i = 0, p = 0;

    for(i = 0; cipher[i] != '\0'; i++) {
        char k;
        for(k = 0 ; k < 64 && base46_map[k] != cipher[i]; k++);
        buffer[counts++] = k;
        if(counts == 4) {
            plain[p++] = (buffer[0] << 2) + (buffer[1] >> 4);
            if(buffer[2] != 64)
                plain[p++] = (buffer[1] << 4) + (buffer[2] >> 2);
            if(buffer[3] != 64)
                plain[p++] = (buffer[2] << 6) + buffer[3];
            counts = 0;
        }
    }

    plain[p] = '\0';    /* string padding character */
    return plain;
}

/*---------------------SMTP发邮件框架--------------------------*/
// 构造函数
bSMTP::bSMTP()
{ 
  m_rec=0;
  m_socket=0; 
  memset(m_from,0,sizeof(m_from));
}

// 连接邮箱服务器
// ipaddr:邮箱域名  port:端口号
// 返回值:0-成功 -1-失败
int bSMTP::connectServer(const char* ipaddr,int port)
{
  if( m_rec != 0 ) return -1;

  // 创建客户端socket
  if ( (m_socket = socket(AF_INET,SOCK_STREAM,0)) == -1 )
  { perror("socket"); return -1; }

  // 连接服务端设备信息(端口号)
  struct sockaddr_in server;
  memset(&server,0,sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  // 解析IP域名,存入结构体IP地址
  struct hostent* h;
  if ( (h = gethostbyname(ipaddr)) == 0 )   // 指定服务端的ip地址
  { printf("gethostbyname failed.\n"); close(m_socket); return -1; }

  memcpy(&server.sin_addr,h->h_addr,h->h_length);
  
  // 建立连接
  // 向服务端发起连接清求
  if (connect(m_socket,(struct sockaddr *)&server,sizeof(server)) != 0)  
  { perror("connect"); close(m_socket); return -1; }
  printf("已连接邮箱服务器(%s)成功\n\n",ipaddr);

  // 接受服务器返回消息
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));
  int iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) 
  {
    printf("Server无返回消息\n");
    return -1;
  }
  printf("Server返回消息:\n%s",buffer);
  if( strncmp(buffer,"220",3) != 0 ) return -1;

  m_rec=1;

  // 发送HELO MSG确认连接
  if( sendHelloMsg() == -1 ) return -1;

  // 发送登录请求
  if( sendAuthLogin() == -1 ) return -1;

  m_rec=3;

  return 0;
}

// 发送报文EHLO MSG确认连接
// 返回值:0-成功 -1-失败
int bSMTP::sendHelloMsg()
{
  char buffer[1024];

  // 发送报文EHLO MSG
  memset(buffer,0,sizeof(buffer));
  strcpy(buffer,"EHLO MSG\r\n");
  int iret=send(m_socket,buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"250",3) != 0 ) return -1;
  
  return 0;
}

// 发送报文AUTH LOGIN登录请求
// 返回值:0-成功 -1-失败
int bSMTP::sendAuthLogin()
{
  char buffer[1024];

  // 发送报文AUTH LOGIN
  memset(buffer,0,sizeof(buffer));
  strcpy(buffer,"AUTH LOGIN\r\n");
  int iret=send(m_socket,buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"334",3) != 0 ) return -1;
  
  return 0;
}

// 发送报文用户名和识别码
// UserName:用户名 Password:识别码
// 返回值:0-成功 -1-失败
int bSMTP::sendAuthInfo(const char* UserName,const char* PassWord)
{
  if( m_rec != 3 ) 
  {
    printf("未连接邮箱服务器\n");
    return -1;
  }

  char buffer[1024];

  // 发送报文Base64编码的用户名
  memset(buffer,0,sizeof(buffer));
  strcpy(buffer,base64_encode(UserName));
  strcat(buffer,"\r\n");
  int iret=send(m_socket,buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"334",3) != 0 ) return -1;

  // 发送报文Base64编码的识别码
  memset(buffer,0,sizeof(buffer));
  strcpy(buffer,base64_encode(PassWord));
  strcat(buffer,"\r\n");
  iret=send(m_socket,buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"235",3) != 0 ) return -1;

  // 将用户名存储于私有变量m_from
  strcpy(m_from,UserName);

  m_rec=4;
  
  return 0;
}

// 发送报文收件人邮箱
// to:收件人邮箱
// 返回值:0-成功 -1-失败
int bSMTP::sendRecipient(const char* to)
{
  if( m_rec != 4 ) 
  {
    printf("未发送用户名和识别码\n");
    return -1;
  }

  char buffer[1024];

  // 发送报文发件人邮箱
  memset(buffer,0,sizeof(buffer));
  sprintf(buffer,"MAIL FROM:<%s>\r\n",m_from);
  int iret=send(m_socket,(unsigned char *)buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"250",3) != 0 ) return -1;

  // 发送报文收件人邮箱
  memset(buffer,0,sizeof(buffer));
  sprintf(buffer,"RCPT TO:<%s>\r\n",to);
  iret=send(m_socket,(unsigned char *)buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"250",3) != 0 ) return -1;

  m_rec=5;
  
  return 0;
}

// 发送报文DATA
// 返回值:0-成功 -1-失败
int bSMTP::sendData()
{
  char buffer[1024];

  // 发送报文DATA开头
  memset(buffer,0,sizeof(buffer));
  strcpy(buffer,"DATA\r\n");
  int iret=send(m_socket,(unsigned char *)buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"354",3) != 0 ) return -1;
  
  return 0;
}

// 发送报文邮件主题和内容
// subject:主题 text:内容
// 返回值:0-成功 -1-失败
int bSMTP::sendMail(const char *subject,const char *text)
{
  if( m_rec != 5 ) 
  {
    printf("未发送收件人邮箱\n");
    return -1;
  }

  if( sendData() == -1 ) return -1;

  char buffer[4096];

  // 发送报文邮件内容
  memset(buffer,0,sizeof(buffer));
  sprintf(buffer,"From:%s\r\nSubject:%s\r\n\r\n%s\r\n\r\n\r\n.\r\n",m_from,subject,text);
  int iret=send(m_socket,(unsigned char *)buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"250",3) != 0 ) return -1;

  if( sendEnd() == -1 ) return -1;

  m_rec=8;
  
  return 0;
}

// 发送报文Quit
// 返回值:0-成功 -1-失败
int bSMTP::sendEnd()
{
  char buffer[1024];

  // 发送报文Quit
  memset(buffer,0,sizeof(buffer));
  strcpy(buffer,"Quit\r\n");
  int iret=send(m_socket,(unsigned char *)buffer,strlen(buffer),0);
  if( iret== -1 ) return -1;

  // 接受服务器报文
  memset(buffer,0,sizeof(buffer));
  iret=recv(m_socket,buffer,sizeof(buffer),0);
  if( iret== -1 ) return -1;
  printf("%s",buffer);
  if( strncmp(buffer,"221",3) != 0 ) return -1;
  
  return 0;
}

// 关闭资源
void bSMTP::Close()
{
  if( m_rec == 8 ) { printf("\n已成功发送邮件\n"); }
  else { printf("\n发送邮件失败\n"); }

  m_rec=0;

  close(m_socket);
  m_socket=0;

  memset(m_from,0,sizeof(m_from));
}

// 析构函数
bSMTP::~bSMTP()
{ Close(); }