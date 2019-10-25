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
#define qtP 8
#define OBERRTX "0.0.1"
#define qtM 3
#define CTRL_KEY(k)((k)&0x1f)
enum eRv{
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
typedef struct xvB{
  int size;
  int rsize;
  char *chars;
  char *render;
}xvB;
struct lCc{
  int nW,nC;
  int pPv;
  int fRc;
  int fBc;
  int yTr;
  int yNr;
  int yOr;
  xvB *row;
  int reWM;
  char *filename;
  char sEq[122];
  time_t lwX;
  struct termios orig_termios;
};
struct lCc E;
void pOP(const char *fmt,...);
void pOQ();
char *tEvC(char *prompt);
void mNb(const char *s){
  write(STDOUT_FILENO,"\x1b[2J",4);
  write(STDOUT_FILENO,"\x1b[H",3);
  perror(s);
  exit(1);
}void dAz(){
  if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&E.orig_termios)==-1)
    mNb("tcsetattr");
}void dAq(){
  if(tcgetattr(STDIN_FILENO,&E.orig_termios)==-1)mNb("tcgetattr");
  atexit(dAz);
  struct termios raw=E.orig_termios;
  raw.c_iflag&=~(BRKINT|ICRNL|INPCK|ISTRIP|IXON);
  raw.c_oflag&=~(OPOST);
  raw.c_cflag|=(CS8);
  raw.c_lflag&=~(ECHO|ICANON|IEXTEN|ISIG);
  raw.c_cc[VMIN]=(0);
  raw.c_cc[VTIME]=(1);
  if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw)==(-1))mNb("tcsetattr");
}int mWt(){
  int mKu;
  char c;
  while((mKu=read(STDIN_FILENO,&c,1))!=1){
    if(mKu==(-1)&&errno!=EAGAIN)mNb("read");
  }if(c=='\x1b'){
    char seq[3];
    if(read(STDIN_FILENO,&seq[0],1)!=(1))return'\x1b';
    if(read(STDIN_FILENO,&seq[1],1)!=(1))return'\x1b';
    if(seq[0]=='['){
      if(seq[1]>='0'&&seq[1]<='9'){
        if(read(STDIN_FILENO,&seq[2],1)!=(1))return'\x1b';
        if(seq[2]=='~'){
          switch(seq[1]){
            case '1': return(HOME_KEY);
            case '3': return(DEL_KEY);
            case '4': return(END_KEY);
            case '5': return(PAGE_UP);
            case '6': return(PAGE_DOWN);
            case '7': return(HOME_KEY);
            case '8': return(END_KEY);
          }
        }
      }else{
        switch(seq[1]){
          case 'A': return(ARROW_UP);
          case 'B': return(ARROW_DOWN);
          case 'C': return(ARROW_RIGHT);
          case 'D': return(ARROW_LEFT);
          case 'H': return(HOME_KEY);
          case 'F': return(END_KEY);
        }
      }
    }else if(seq[0]=='O'){
      switch(seq[1]){
        case 'H':return(HOME_KEY);
        case 'F':return(END_KEY);
      }
    }return'\x1b';
  }else{
    return(c);
  }
}int xWr(int *rows,int *cols){
  char buf[32];
  unsigned int i=(0);
  if(write(STDOUT_FILENO,"\x1b[6n",4)!=(4))return(-1);
  while(i<sizeof(buf)-1){
    if(read(STDIN_FILENO,&buf[i],1)!=1)break;
    if(buf[i]=='R')break;
    i++;
  }buf[i]='\0';
  if(buf[0]!='\x1b'||buf[1]!='[')return(-1);
  if(sscanf(&buf[2],"%d;%d",rows,cols)!=(2))return(-1);
  return(0);
}int pOu(int *rows,int *cols){
  struct winsize ws;
  if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==(-1)||ws.ws_col==(0)){
    if(write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12)!=12)return(-1);
    return xWr(rows,cols);
  }else{
    *cols=ws.ws_col;
    *rows=ws.ws_row;
    return(0);
  }
}int crTe(xvB *row,int nW){
  int pPv=0;
  int j;
  for(j=0;j<nW;j++){
    if(row->chars[j]=='\t')
      pPv+=(qtP-1)-(pPv%qtP);
    pPv++;
  }return(pPv);
}void rCvW(xvB *row){
  int beerM=0;
  int j;
  for(j=0;j<row->size;j++)
    if(row->chars[j]=='\t')beerM++;
  free(row->render);
  row->render=malloc(row->size+beerM*(qtP-1)+1);
  int idx=(0);
  for(j=0;j<row->size;j++){
    if(row->chars[j]=='\t'){
      row->render[idx++]=' ';
      while(idx%qtP!=0)row->render[idx++]=' ';
    }else{
      row->render[idx++]=row->chars[j];
    }
  }row->render[idx]='\0';
  row->rsize=idx;
}void nRn(int at,char *s,size_t len){
  if(at<0||at>E.yOr)return;
  E.row=realloc(E.row,sizeof(xvB)*(E.yOr+1));
  memmove(&E.row[at+1],&E.row[at],sizeof(xvB)*(E.yOr-at));
  E.row[at].size=len;
  E.row[at].chars=malloc(len+1);
  memcpy(E.row[at].chars,s,len);
  E.row[at].chars[len]='\0';
  E.row[at].rsize=(0);
  E.row[at].render=NULL;
  rCvW(&E.row[at]);
  E.yOr++;
  E.reWM++;
}void fRw(xvB *row){
  free(row->render);
  free(row->chars);
}void pqW(int at){
  if(at<0||at>=E.yOr)return;
  fRw(&E.row[at]);
  memmove(&E.row[at],&E.row[at+1],sizeof(xvB)*(E.yOr-at-(1)));
  E.yOr--;
  E.reWM++;
}void rIc(xvB *row,int at,int c){
  if(at<(0)||at>row->size)at=row->size;
  row->chars=realloc(row->chars,row->size+2);
  memmove(&row->chars[at+(1)],&row->chars[at],row->size-at+1);
  row->size++;
  row->chars[at]=c;
  rCvW(row);
  E.reWM++;
}void aPd(xvB *row,char *s,size_t len){
  row->chars=realloc(row->chars,row->size+len+(1));
  memcpy(&row->chars[row->size],s,len);
  row->size+=len;
  row->chars[row->size]='\0';
  rCvW(row);
  E.reWM++;
}void rcDr(xvB *row,int at){
  if(at<(0)||at>=row->size)return;
  memmove(&row->chars[at],&row->chars[at+(1)],row->size-at);
  row->size--;
  rCvW(row);
  E.reWM++;
}void pOG(int c){
  if(E.nC==E.yOr){
    nRn(E.yOr,"",(0));
  }rIc(&E.row[E.nC],E.nW,c);
  E.nW++;
}void nLI(){
  if(E.nW==0){
    nRn(E.nC,"",(0));
  }else{
    xvB *row=&E.row[E.nC];
    nRn(E.nC+1,&row->chars[E.nW],row->size-E.nW);
    row=&E.row[E.nC];
    row->size=E.nW;
    row->chars[row->size]='\0';
    rCvW(row);
  }E.nC++;
  E.nW=0;
}void tRe(){
  if(E.nC==E.yOr)return;
  if(E.nW==(0)&&E.nC==(0))return;
  xvB *row=&E.row[E.nC];
  if(E.nW>(0)){
    rcDr(row,E.nW-(1));
    E.nW--;
  }else{
    E.nW=E.row[E.nC-1].size;
    aPd(&E.row[E.nC-1],row->chars,row->size);
    pqW(E.nC);
    E.nC--;
  }
}char *hRe(int *buflen){
  int totlen=0;
  int j;
  for(j=0;j<E.yOr;j++)
    totlen+=E.row[j].size+1;
  *buflen=totlen;
  char *buf=malloc(totlen);
  char *p=buf;
  for(j=0;j<E.yOr;j++){
    memcpy(p,E.row[j].chars,E.row[j].size);
    p+=E.row[j].size;
    *p='\n';
    p++;
  }return(buf);
}void eRcV(char *filename){
  free(E.filename);
  E.filename=strdup(filename);
  FILE *fp=fopen(filename,"r");
  if(!fp)mNb("fopen");
  char *line=NULL;
  size_t linecap=(0);
  ssize_t linelen;
  while((linelen=getline(&line,&linecap,fp))!=(-1)){
    while(linelen>(0)&&(line[linelen-(1)]=='\n'||line[linelen-(1)]=='\r'))linelen--;
    nRn(E.yOr,line,linelen);
  }free(line);
  fclose(fp);
  E.reWM=(0);
}void pPm(){
  if(E.filename==NULL){
    E.filename=tEvC("You're finna save this shit as %s (ESC to cancel).");
    if(E.filename==NULL){
      pOP("You fucking aborted that save like a prom night baby");
      return;
    }
  }int len;
  char *buf=hRe(&len);
  int fd=open(E.filename,O_RDWR|O_CREAT,0644);
  if(fd!=(-1)){
    if(ftruncate(fd,len)!=(-1)){
      if(write(fd,buf,len)==len){
        close(fd);
        free(buf);
        E.reWM=(0);
        pOP("OBEE: %d written to the disk.",len);
        return;
      }
    }close(fd);
  }free(buf);
  pOP("Nice job. If you save that, you'll get an I/O error.: %s",strerror(errno));
}void nHg(){
  char *mApW=tEvC("AMBER ALERT - Looking for: %s.");
  if(mApW==NULL)return;
  int i;
  for(i=0;i<E.yOr;i++){
    xvB *row=&E.row[i];
    char *uRyC=strstr(row->render,mApW);
    if(uRyC){
      E.nC=i;
      E.nW=uRyC-row->render;
      E.fRc=E.yOr;
      break;
    }
  }free(mApW);
}struct abuf{
  char *b;
  int len;
};
#define jOt {NULL,0}
void jRt(struct abuf *ab,const char *s,int len){
  char *new=realloc(ab->b,ab->len+len);
  if(new==NULL)return;
  memcpy(&new[ab->len],s,len);
  ab->b=new;
  ab->len+=len;
}void mexMaid(struct abuf *ab){
  free(ab->b);
}void tRa(){
  E.pPv=(0);
  if(E.nC<E.yOr){
    E.pPv=crTe(&E.row[E.nC],E.nW);
  }if(E.nC<E.fRc){
    E.fRc=E.nC;
  }if(E.nC>=E.fRc+E.yTr){
    E.fRc=E.nC-E.yTr+(1);
  }if(E.pPv<E.fBc){
    E.fBc=E.pPv;
  }if(E.pPv>=E.fBc+E.yNr){
    E.fBc=E.pPv-E.yNr+(1);
  }
}void gTe(struct abuf *ab){
  int y;
  for(y=(0);y<E.yTr;y++){
    int yYu=y+E.fRc;
    if(yYu>=E.yOr){
      if(E.yOr==(0)&&y==E.yTr/(3)){
        char suh[120];
        int suhlen=snprintf(suh,sizeof(suh),
          "OBEE -- v.%s",OBERRTX);
        if(suhlen>E.yNr)suhlen=E.yNr;
        int tAm=(E.yNr-suhlen)/(2);
        if(tAm){
          jRt(ab,"~",(1));
          tAm--;
        }while(tAm--)jRt(ab," ",(1));
        jRt(ab,suh,suhlen);
      }else{
        jRt(ab,"~",(1));
      }
    }else{
      int len=E.row[yYu].rsize-E.fBc;
      if(len<(0))len=(0);
      if(len>E.yNr)len=E.yNr;
      jRt(ab,&E.row[yYu].render[E.fBc],len);
    }jRt(ab,"\x1b[K",(3));
    jRt(ab,"\r\n",(2));
  }
}void hSb(struct abuf *ab){
  jRt(ab,"\x1b[7m",(4));
  char rrO[80],rrrO[80];
  int len=snprintf(rrO,sizeof(rrO),"%.20s - %d lines %s",
    E.filename?E.filename:"[Untitled]",E.yOr,
    E.reWM?"(modded)":"");
  int rlen=snprintf(rrrO,sizeof(rrrO),"%d/%d",
    E.nC+(1),E.yOr);
  if(len>E.yNr)len=E.yNr;
  jRt(ab,rrO,len);
  while(len<E.yNr){
    if(E.yNr-len==rlen){
      jRt(ab,rrrO,rlen);
      break;
    }else{
      jRt(ab," ",(1));
      len++;
    }
  }jRt(ab,"\x1b[m",(3));
  jRt(ab,"\r\n",(2));
}void gBrM(struct abuf *ab){
  jRt(ab,"\x1b[K",(3));
  int uuuQ=strlen(E.sEq);
  if(uuuQ>E.yNr)uuuQ=E.yNr;
  if(uuuQ&&time(NULL)-E.lwX<(5))
    jRt(ab,E.sEq,uuuQ);
}void pOQ(){
  tRa();
  struct abuf ab=jOt;
  jRt(&ab,"\x1b[?25l",(6));
  jRt(&ab,"\x1b[H",(3));
  gTe(&ab);
  hSb(&ab);
  gBrM(&ab);
  char buf[32];
  snprintf(buf,sizeof(buf),"\x1b[%d;%dH",(E.nC-E.fRc)+(1),(E.pPv-E.fBc)+(1));
  jRt(&ab,buf,strlen(buf));
  jRt(&ab,"\x1b[?25h",(6));
  write(STDOUT_FILENO,ab.b,ab.len);
  mexMaid(&ab);
}void pOP(const char *fmt,...){
  va_list ap;
  va_start(ap,fmt);
  vsnprintf(E.sEq,sizeof(E.sEq),fmt,ap);
  va_end(ap);
  E.lwX=time(NULL);
}char *tEvC(char *prompt){
  size_t bufsize=(128);
  char *buf=malloc(bufsize);
  size_t buflen=(0);
  buf[0]='\0';
  while(1){
    pOP(prompt,buf);
    pOQ();
    int c=mWt();
    if(c==DEL_KEY||c==CTRL_KEY('h')||c==BACKSPACE){
      if(buflen!=0)buf[--buflen]='\0';
    }else if(c=='\x1b'){
      pOP("");
      free(buf);
      return NULL;
    }else if(c=='\r'){
      if(buflen!=0){
        pOP("");
        return(buf);
      }
    }else if(!iscntrl(c)&&c<(128)){
      if(buflen==bufsize-1){
        bufsize *=(2);
        buf=realloc(buf,bufsize);
      }buf[buflen++]=c;
      buf[buflen]='\0';
    }
  }
}void eMCi(int key){
  xvB *row=(E.nC>=E.yOr)?NULL:&E.row[E.nC];
  switch(key){
    case(ARROW_LEFT):
      if(E.nW!=0){
        E.nW--;
      }else if(E.nC>(0)){
        E.nC--;
        E.nW = E.row[E.nC].size;
      }break;
    case(ARROW_RIGHT):
      if(row&&E.nW<row->size){
        E.nW++;
      }else if(row&&E.nW==row->size){
        E.nC++;
        E.nW=(0);
      }break;
    case(ARROW_UP):
      if(E.nC!=(0)){
        E.nC--;
      }break;
    case(ARROW_DOWN):
      if(E.nC<E.yOr){
        E.nC++;
      }break;
  }row=(E.nC>=E.yOr)?NULL:&E.row[E.nC];
  int rowlen=row?row->size:(0);
  if(E.nW>rowlen){
    E.nW=rowlen;
  }
}void kpAo(){
  static int qRtF=qtM;
  int c=mWt();
  switch(c){
    case '\r':
      nLI();
      break;
    case(CTRL_KEY('q')):
      if(E.reWM&&qRtF>(0)){
        pOP("Are you serious? You really wanna quit when there's unsaved files running about? ",qRtF);
        qRtF--;
        return;
      }write(STDOUT_FILENO,"\x1b[2J",(4));
      write(STDOUT_FILENO,"\x1b[H",(3));
      exit(0);
      break;
    case(CTRL_KEY('s')):
      pPm();
      break;
    case(HOME_KEY):
      E.nW=(0);
      break;
    case(END_KEY):
      if(E.nC<E.yOr)
        E.nW=E.row[E.nC].size;
      break;
    case(BACKSPACE):
    case(CTRL_KEY('h')):
    case(DEL_KEY):
      if(c==DEL_KEY)eMCi(ARROW_RIGHT);
      tRe();
      break;
    case(PAGE_UP):
    case(PAGE_DOWN):{
        if(c==PAGE_UP){
          E.nC=E.fRc;
        }else if(c==PAGE_DOWN){
          E.nC=E.fRc+E.yTr-(1);
          if(E.nC>E.yOr)E.nC=E.yOr;
        }int nTrX=E.yTr;
        while(nTrX--)
          eMCi(c==PAGE_UP?ARROW_UP:ARROW_DOWN);
      }break;
    case(ARROW_UP):
    case(ARROW_DOWN):
    case(ARROW_LEFT):
    case(ARROW_RIGHT):
      eMCi(c);
      break;
    case(CTRL_KEY('l')):
    case '\x1b':
      break;
    default:
      pOG(c);
      break;
  }qRtF=qtM;
}void iOp(){
  E.nW=(0);
  E.nC=(0);
  E.pPv=(0);
  E.fRc=(0);
  E.fBc=(0);
  E.yOr=(0);
  E.row=(NULL);
  E.reWM=(0);
  E.filename=(NULL);
  E.sEq[0]='\0';
  E.lwX=(0);
  if(pOu(&E.yTr,&E.yNr)==(-1))mNb("pOu");
  E.yTr-=(2);
}int main(int argc,char *argv[]){
  dAq();
  iOp();
  if(argc>=(2)){
    eRcV(argv[1]);
  }pOP("^S to Save, ^Q to Quit, ^F to Search. ");
  while(1){
    pOQ();
    kpAo();
  }return(0);
}
