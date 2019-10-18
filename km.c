#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#define verX "0.0.1"
#define OBEERTX 8
#define qtO 3
#define CTRL_KEY(k)((k)&0x1f)
enum mSU{
  BACKSPACE=127,
  ARROW_LEFT=1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};
typedef struct erow{
  int size;
  int rsize;
  char *chars;
  char *render;
}erow;
struct cHO{
  int rtpX,rtpM;
  int rx;
  int rOz;
  int cOz;
  int rTz;
  int cTz;
  int rNz;
  erow *row;
  int xVb;
  char *filename;
  char srT[80];
  time_t srT_time;
  struct termios orig_termios;
};
struct cHO E;
void xxxE(const char *fmt,...);
void xxxR();
char *xxxP(char *prompt);
void rtVbw(const char *s){
  write(STDOUT_FILENO,"\x1b[2J",4);
  write(STDOUT_FILENO,"\x1b[H",3);
  perror(s);
  exit(1);
}void cccR(){
  if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&E.orig_termios)==-1)
    rtVbw("tcsetattr");
}void xccR(){
  if(tcgetattr(STDIN_FILENO,&E.orig_termios)==-1)rtVbw("tcgetattr");
  atexit(cccR);
  struct termios raw=E.orig_termios;
  raw.c_iflag&=~(BRKINT|ICRNL|INPCK|ISTRIP|IXON);
  raw.c_oflag&=~(OPOST);
  raw.c_cflag|=(CS8);
  raw.c_lflag&=~(ECHO|ICANON|IEXTEN|ISIG);
  raw.c_cc[VMIN]=0;
  raw.c_cc[VTIME]=1;
  if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw)==-1)rtVbw("tcsetattr");
}int rennmS(){
  int nread;
  char c;
  while((nread=read(STDIN_FILENO,&c,1))!=1){
    if(nread==-1&&errno!=EAGAIN)rtVbw("read");
  }if(c=='\x1b'){
    char seq[3];
    if(read(STDIN_FILENO,&seq[0],1)!=1)return('\x1b');
    if(read(STDIN_FILENO,&seq[1],1)!=1)return('\x1b');
    if(seq[0]=='['){
      if(seq[1]>='0'&&seq[1]<='9'){
        if(read(STDIN_FILENO,&seq[2],1)!=1)return('\x1b');
        if(seq[2]=='~'){
          switch(seq[1]){
            case '1':return(HOME_KEY);
            case '3':return(DEL_KEY);
            case '4':return(END_KEY);
            case '5':return(PAGE_UP);
            case '6':return(PAGE_DOWN);
            case '7':return(HOME_KEY);
            case '8':return(END_KEY);
          }
        }
      }else{
        switch(seq[1]){
          case 'A':return(ARROW_UP);
          case 'B':return(ARROW_DOWN);
          case 'C':return(ARROW_RIGHT);
          case 'D':return(ARROW_LEFT);
          case 'H':return(HOME_KEY);
          case 'F':return(END_KEY);
        }
      }
    }else if(seq[0]=='O'){
      switch(seq[1]){
        case 'H': return(HOME_KEY);
        case 'F': return(END_KEY);
      }
    }return('\x1b');
  }else{
    return(c);
  }
}int xcp(int *rows,int *cols){
  char buf[32];
  unsigned int i=0;
  if(write(STDOUT_FILENO,"\x1b[6n",4)!=4)return(-1);
  while(i<sizeof(buf)-1){
    if(read(STDIN_FILENO,&buf[i],1)!=1)break;
    if(buf[i]=='R')break;
    i++;
  }buf[i]='\0';
  if(buf[0]!='\x1b'||buf[1]!='[')return(-1);
  if(sscanf(&buf[2],"%d;%d",rows,cols)!=2)return(-1);
  return(0);
}int wsv(int *rows,int *cols){
  struct winsize ws;
  if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==-1||ws.ws_col==0){
    if(write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12)!=12) return(-1);
    return xcp(rows,cols);
  }else{
    *cols=ws.ws_col;
    *rows=ws.ws_row;
    return(0);
  }
}int eRCR(erow *row,int rtpX){
  int rx=0;
  int j;
  for(j=0;j<rtpX;j++){
    if(row->chars[j]=='\t')
      rx+=(OBEERTX-1)-(rx%OBEERTX);
    rx++;
  }return(rx);
}void eURx(erow *row){
  int tabs=0;
  int j;
  for(j=0;j<row->size;j++)
    if(row->chars[j]=='\t')tabs++;
  free(row->render);
  row->render=malloc(row->size+tabs*(OBEERTX-1)+1);
  int idx=0;
  for(j=0;j<row->size;j++){
    if(row->chars[j]=='\t'){
      row->render[idx++]=' ';
      while(idx%OBEERTX!=0)row->render[idx++]=' ';
    }else{
      row->render[idx++]=row->chars[j];
    }
  }row->render[idx]='\0';
  row->rsize=idx;
}void insRIR(int at,char *s,size_t len){
  if(at<0||at>E.rNz)return;
  E.row=realloc(E.row,sizeof(erow)*(E.rNz+1));
  memmove(&E.row[at+1],&E.row[at],sizeof(erow)*(E.rNz-at));
  E.row[at].size=len;
  E.row[at].chars=malloc(len+1);
  memcpy(E.row[at].chars,s,len);
  E.row[at].chars[len]='\0';
  E.row[at].rsize=0;
  E.row[at].render=NULL;
  eURx(&E.row[at]);
  E.rNz++;
  E.xVb++;
}void eFRx(erow *row){
  free(row->render);
  free(row->chars);
}void insDe(int at){
  if(at<0||at>=E.rNz)return;
  eFRx(&E.row[at]);
  memmove(&E.row[at],&E.row[at+1],sizeof(erow)*(E.rNz-at-1));
  E.rNz--;
  E.xVb++;
}void insR(erow *row,int at,int c){
  if(at<0||at>row->size)at=row->size;
  row->chars=realloc(row->chars,row->size+2);
  memmove(&row->chars[at+1],&row->chars[at],row->size-at+1);
  row->size++;
  row->chars[at]=c;
  eURx(row);
  E.xVb++;
}void insRA(erow *row,char *s,size_t len){
  row->chars=realloc(row->chars,row->size+len+1);
  memcpy(&row->chars[row->size],s,len);
  row->size+=len;
  row->chars[row->size]='\0';
  eURx(row);
  E.xVb++;
}void eRDc(erow *row, int at) {
  if(at<0||at>=row->size)return;
  memmove(&row->chars[at],&row->chars[at+1],row->size-at);
  row->size--;
  eURx(row);
  E.xVb++;
}void vGh(int c){
  if(E.rtpM==E.rNz){
    insRIR(E.rNz,"",0);
  }insR(&E.row[E.rtpM],E.rtpX,c);
  E.rtpX++;
}void insNL(){
  if(E.rtpX==0){
    insRIR(E.rtpM,"",0);
  }else{
    erow *row=&E.row[E.rtpM];
    insRIR(E.rtpM+1,&row->chars[E.rtpX],row->size-E.rtpX);
    row=&E.row[E.rtpM];
    row->size=E.rtpX;
    row->chars[row->size]='\0';
    eURx(row);
  }E.rtpM++;
  E.rtpX=0;
}void eDrtpXx(){
  if(E.rtpM==E.rNz)return;
  if(E.rtpX==0&&E.rtpM==0)return;
  erow *row=&E.row[E.rtpM];
  if(E.rtpX>0){
    eRDc(row,E.rtpX-1);
    E.rtpX--;
  }else{
    E.rtpX=E.row[E.rtpM-1].size;
    insRA(&E.row[E.rtpM-1],row->chars,row->size);
    insDe(E.rtpM);
    E.rtpM--;
  }
}char *erXXw(int *buflen){
  int totlen=0;
  int j;
  for(j=0;j<E.rNz;j++)
    totlen+=E.row[j].size+1;
  *buflen=totlen;
  char *buf=malloc(totlen);
  char *p=buf;
  for(j=0;j<E.rNz;j++){
    memcpy(p,E.row[j].chars,E.row[j].size);
    p+=E.row[j].size;
    *p='\n';
    p++;
  }return(buf);
}void oWScV(char *filename){
  free(E.filename);
  E.filename=strdup(filename);
  FILE *fp=fopen(filename,"r");
  if(!fp)rtVbw("fopen");
  char *line=NULL;
  size_t linecap=0;
  ssize_t linelen;
  while((linelen=getline(&line,&linecap,fp))!=-1){
    while(linelen>0&&(line[linelen-1]=='\n'||line[linelen-1=='\r'))
      linelen--;
    insRIR(E.rNz,line,linelen);
  }free(line);
  fclose(fp);
  E.xVb=0;
}void xxxSA(){
  if(E.filename==NULL){
    E.filename=xxxP("Saving as: %s (ESC to cancel)");
    if(E.filename==NULL){
      xxxE("Fuck, you can't do anything right, can you? You can't even save things correctly.");
      return;
    }
  }int len;
  char *buf=erXXw(&len);
  int fd=open(E.filename,O_RDWR|O_CREAT,0644);
  if(fd!=-1){
    if(ftruncate(fd,len)!=-1){
      if(write(fd,buf,len)==len){
        close(fd);
        free(buf);
        E.xVb=0;
        xxxE("%d bytes written to disk(s).",len);
        return;
      }
    }close(fd);
  }free(buf);
  xxxE("You can't save that. Here's why: %s", strerror(errno));
}struct xbN{
  char *b;
  int len;
};
#define yyytE {NULL,0}
void rrrWv(struct xbN *ab,const char *s,int len){
  char *new=realloc(ab->b,ab->len+len);
  if(new==NULL)return;
  memcpy(&new[ab->len],s,len);
  ab->b=new;
  ab->len+=len;
}void wwwF(struct xbN *ab){
  free(ab->b);
}void xSr(){
  E.rx=0;
  if(E.rtpM<E.rNz){
    E.rx=eRCR(&E.row[E.rtpM],E.rtpX);
  }if(E.rtpM<E.rOz){
    E.rOz=E.rtpM;
  }if(E.rtpM>=E.rOz+E.rTz){
    E.rOz=E.rtpM-E.rTz+1;
  }if(E.rx<E.cOz){
    E.cOz=E.rx;
  }if(E.rx>=E.cOz+E.cTz){
    E.cOz=E.rx-E.cTz+1;
  }
}void eeeMR(struct xbN *ab){
  int y;
  for(y=0;y<E.rTz;y++){
    int trEE=y+E.rOz;
    if(trEE>=E.rNz){
      if(E.rNz==0&&y==E.rTz/3){
        char rXCb[80];
        int rXCblen=snprintf(rXCb,sizeof(rXCb),
          "OBEE -- v. %s",verX);
        if(rXCblen>E.cTz)rXCblen=E.cTz;
        int BRSa=(E.cTz-rXCblen)/2;
        if(BRSa){
          rrrWv(ab,"~",1);
          BRSa--;
        }while(BRSa--)rrrWv(ab," ",1);
        rrrWv(ab,rXCb,rXCblen);
      }else{
        rrrWv(ab,"~",1);
      }
    }else{
      int len=E.row[trEE].rsize-E.cOz;
      if(len<0)len=0;
      if(len>E.cTz)len=E.cTz;
      rrrWv(ab,&E.row[trEE].render[E.cOz],len);
    }rrrWv(ab,"\x1b[K",3);
    rrrWv(ab,"\r\n",2);
  }
}void sKj(struct xbN *ab){
  rrrWv(ab,"\x1b[7m",4);
  char rWq[80],rrWq[80];
  int len=snprintf(rWq,sizeof(rWq),"%.20s - %d lines %s",
    E.filename?E.filename:"[No Name]",E.rNz,
    E.xVb?"(modded)":"");
  int rlen=snprintf(rrWq,sizeof(rrWq),"%d/%d",
    E.rtpM+1,E.rNz);
  if(len>E.cTz)len=E.cTz;
  rrrWv(ab,rWq,len);
  while(len<E.cTz){
    if(E.cTz-len==rlen){
      rrrWv(ab,rrWq,rlen);
      break;
    }else{
      rrrWv(ab," ",1);
      len++;
    }
  }rrrWv(ab,"\x1b[m",3);
  rrrWv(ab,"\r\n",2);
}void rEwvc(struct xbN *ab){
  rrrWv(ab,"\x1b[K",3);
  int msglen=strlen(E.srT);
  if(msglen>E.cTz)msglen=E.cTz;
  if(msglen&&time(NULL)-E.srT_time<5)
    rrrWv(ab,E.srT,msglen);
}void xxxR(){
  xSr();
  struct xbN ab=yyytE;
  rrrWv(&ab,"\x1b[?25l",6);
  rrrWv(&ab,"\x1b[H",3);
  eeeMR(&ab);
  sKj(&ab);
  rEwvc(&ab);
  char buf[32];
  snprintf(buf,sizeof(buf),"\x1b[%d;%dH",(E.rtpM-E.rOz)+1,(E.rx-E.cOz)+1);
  rrrWv(&ab,buf,strlen(buf));
  rrrWv(&ab,"\x1b[?25h",6);
  write(STDOUT_FILENO,ab.b,ab.len);
  wwwF(&ab);
}void xxxE(const char *fmt,...){
  va_list ap;
  va_start(ap,fmt);
  vsnprintf(E.srT, sizeof(E.srT),fmt,ap);
  va_end(ap);
  E.srT_time=time(NULL);
}char *xxxP(char *prompt){
  size_t bufsize=128;
  char *buf=malloc(bufsize);
  size_t buflen=0;
  buf[0]='\0';
  while(1){
    xxxE(prompt,buf);
    xxxR();
    int c=rennmS();
    if(c==DEL_KEY||c==CTRL_KEY('h')||c==BACKSPACE){
      if(buflen!=0)buf[--buflen]='\0';
    }else if(c=='\x1b'){
      xxxE("");
      free(buf);
      return(NULL);
    }else if(c=='\r'){
      if(buflen!=0){
        xxxE("");
        return(buf);
      }
    }else if(!iscntrl(c)&&c<128){
      if(buflen==bufsize-1){
        bufsize(*)=2;
        buf=realloc(buf,bufsize);
      }buf[buflen++]=c;
      buf[buflen]='\0';
    }
  }
}void nyRE(int key){
  erow *row=(E.rtpM>=E.rNz)?NULL:&E.row[E.rtpM];
  switch(key){
    case ARROW_LEFT:
      if(E.rtpX!=0){
        E.rtpX--;
      }else if(E.rtpM>0){
        E.rtpM--;
        E.rtpX=E.row[E.rtpM].size;
      }break;
    case ARROW_RIGHT:
      if(row&&E.rtpX<row->size){
        E.rtpX++;
      }else if(row&&E.rtpX==row->size){
        E.rtpM++;
        E.rtpX=0;
      }break;
    case ARROW_UP:
      if(E.rtpM!=0){
        E.rtpM--;
      }break;
    case ARROW_DOWN:
      if(E.rtpM<E.rNz){
        E.rtpM++;
      }break;
  }row=(E.rtpM>=E.rNz)?NULL:&E.row[E.rtpM];
  int rowlen=row?row->size:0;
  if(E.rtpX>rowlen){
    E.rtpX=rowlen;
  }
}void whore(){
  static int rgdrIU=qtO;
  int c=rennmS();
  switch(c){
    case '\r':
      insNL();
      break;
    case CTRL_KEY('q'):
      if(E.xVb&&rgdrIU>0){
        xxxE("Sure, pal. You can do that. But you won't save anything. "
          "Press Ctrl-Q %d more times to quit.",rgdrIU);
        rgdrIU--;
        return;
      }write(STDOUT_FILENO,"\x1b[2J",4);
      write(STDOUT_FILENO,"\x1b[H",3);
      exit(0);
      break;
    case CTRL_KEY('s'):
      xxxSA();
      break;
    case HOME_KEY:
      E.rtpX=0;
      break;
    case END_KEY:
      if(E.rtpM<E.rNz)
        E.rtpX=E.row[E.rtpM].size;
      break;
    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL_KEY:
      if(c==DEL_KEY)nyRE(ARROW_RIGHT);
      eDrtpXx();
      break;
    case PAGE_UP:
    case PAGE_DOWN:{
        if(c==PAGE_UP){
          E.rtpM=E.rOz;
        }else if(c==PAGE_DOWN){
          E.rtpM=E.rOz+E.rTz-1;
          if(E.rtpM>E.rNz)E.rtpM=E.rNz;
        }int times=E.rTz;
        while(times--)
          nyRE(c==PAGE_UP?ARROW_UP:ARROW_DOWN);
      }break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      nyRE(c);
      break;
    case CTRL_KEY('l'):
    case '\x1b':
      break;
    default:
      vGh(c);
      break;
  }rgdrIU=qtO;
}void eiEh(){
  E.rtpX=0;
  E.rtpM=0;
  E.rx=0;
  E.rOz=0;
  E.cOz=0;
  E.rNz=0;
  E.row=NULL;
  E.xVb=0;
  E.filename=NULL;
  E.srT[0]='\0';
  E.srT_time=0;
  if(wsv(&E.rTz,&E.cTz)==-1)rtVbw("wsv");
  E.rTz-=2;
}int main(int argc, char *argv[]){
  xccR();
  eiEh();
  if(argc>=2){
    oWScV(argv[1]);
  }xxxE("^S means Save. ^Q means Quit. You absolute moron, you. ");
  while(1){
    xxxR();
    whore();
  }return(0);
}
