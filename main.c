#include <windows.h>
#include <string.h>
#include <stdio.h>

#define ISCHECKED(_h_)           SNDMSG(_h_,BM_GETCHECK,0,0)==BST_CHECKED
#define WSV        WS_CHILD|WS_VISIBLE
#define SETFONT(_h_) SendMessage(_h_, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(1, 0))
#define SMS(_h_,_s_) SNDMSG(_h_,WM_SETTEXT,0,(LPARAM)_s_)

char szClassName[ ] = "WinCOpyDeleter";

DWORD CRC32(DWORD start,UCHAR *buf, DWORD len);

HINSTANCE ins;
HWND hWnd,B_go,B_restore,B_same; UINT step = 0;
HWND hAll,hCopy,hError;
UCHAR dropped[MAX_PATH];
UCHAR BatCopy[MAX_PATH];

HANDLE STOP_WORKING;  //event to start and stop
UCHAR IS_RUNNING = 0;

typedef struct LIST_
{
    UCHAR *path;
    DWORD hash;
    DWORD size;
    struct LIST_ *next;
}LIST;
DWORD listLen = 0;

FILE *sss=NULL;
LIST *list=NULL;
LIST *PM; 

DWORD nAll=0;
DWORD nCopy=0;
DWORD nError=0;

//return PATH delete it
//return NULL dont delete it
UCHAR *CheckList(UCHAR *path, DWORD hash,DWORD size)
{
    LIST *s, *p;     
    if(list==NULL)
    {
        list=(LIST*)malloc(sizeof(LIST)); //first call add it
        if(list==NULL)
           return NULL;
        list->path = (UCHAR*)malloc(strlen(path)+2);
        if(list->path==NULL)
        {
             free(list);
             list = NULL;                 
             return NULL; //on error do not delete the file, just continue
        }
        sprintf(list->path,"%s\0",path);
        list->hash = hash; 
        list->size = size; 
        list->next=NULL;
        if(listLen==0)
           PM = list;
    }
    else
    {  //check if is in the list   

       for(p=PM;p!=NULL;p=p->next)
       {   
          if(p->size == size)
          {
             if(p->hash == hash)
            return p->path;
          }
       }
       //I guess didnt find it
       s=(LIST*)malloc(sizeof(LIST));
       if(s==NULL)
          return NULL;
       s->path = (UCHAR*)malloc(strlen(path)+2);
       if(s->path==NULL)
       {
           free(s);                
           return NULL; //on error do not delete the file, just continue
       }
       sprintf(s->path,"%s\0",path);
       s->hash = hash;         
       s->size = size;         
       s->next=NULL;       
       list->next=s;
       list = s;
    }
    listLen++;
 return NULL;
}  

UCHAR buffer[4096];
void AddToTableOrDelete(UCHAR *FileName, BOOL NeedRestore)
{
    HANDLE fd; UCHAR *p;
    DWORD rd=4096;
    BOOL ret;  
    
    DWORD hash=0; DWORD fSize = 0;
    nAll++;
    sprintf(buffer,"%d\0",nAll); SMS(hAll,buffer);
    fd = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if(fd == INVALID_HANDLE_VALUE)
    {
       return;
    }
    fSize = GetFileSize(fd,NULL);
    if((fSize > 0x6400000) || (fSize==0) ) //100 Mb limit or if ZERO size
    {
       CloseHandle(fd); 
       return;
    }           
    while(1)
    {
        ret=ReadFile(fd, buffer, 4096, &rd, NULL);
        if(ret==FALSE || rd==0)
           break;
        buffer[rd]=0;
        hash = CRC32(hash,buffer, rd);
    }  
    CloseHandle(fd); 
    p=CheckList(FileName,hash,fSize);
    if(p != NULL) //delete it
    {
        if((NeedRestore == TRUE) && (sss==NULL) )
        {
           sss = fopen(BatCopy, "wb");
           if(!sss)
           {
               MessageBox(hWnd,"Can't Create RESTORE.BAT","ERROR", MB_ICONINFORMATION|MB_SYSTEMMODAL|MB_OK);
			   exit(0);
			   return;                
           }              
        }
        if(sss) fprintf(sss,"copy /B /Y \"%s\" \"%s\"\r\n", p+step,FileName+step);
                
        SetFileAttributes(FileName,FILE_ATTRIBUTE_NORMAL);
        
        if(DeleteFile(FileName)==0)
        { 
            nError++; sprintf(buffer,"%d\0",nError); SMS(hError,buffer);
        }
        else
        {
            nCopy++; sprintf(buffer,"%d\0",nCopy); SMS(hCopy,buffer);
        }
    }
}

void FreeList()
{
    LIST *s,*prev; 
    for(s=PM;s!=NULL;)
    {
        prev = s;
        if(s->path) free(s->path);
        s=s->next;
        free(prev);                       
    } 
    free(list);
    listLen = 0;
    list=NULL;
}

void ShowShortPath(UCHAR *name)
{
     UCHAR SHpath[MAX_PATH];
     memset(SHpath,0,MAX_PATH);
     if(GetShortPathName(name,SHpath,MAX_PATH) != ERROR_INVALID_PARAMETER)
     {
        SetWindowText(hWnd,SHpath);                                         
     }
     else{ SetWindowText(hWnd,name); }
}

int ScanThisDir(UCHAR *Path, BOOL NeedRestore)
{
	HANDLE h;
	WIN32_FIND_DATA info;
    unsigned char SearchPath[MAX_PATH]; 
    unsigned char CurrentFile[MAX_PATH]; 
    int ret;
    memset(SearchPath,0,MAX_PATH);
    memset(CurrentFile,0,MAX_PATH);
    
    sprintf(SearchPath,"%s\\*\0",Path);
	h = FindFirstFile(SearchPath, &info);
	if(h == INVALID_HANDLE_VALUE)  return -1;
	ShowShortPath(Path);	
    while(FindNextFile(h, &info) != 0)
    {
        if(WaitForSingleObject(STOP_WORKING,0)==0)
               break;
        if(strcmp(info.cFileName, ".") == 0){  continue; }
        if(strcmp(info.cFileName, "..") == 0){ continue; }
        
        sprintf(CurrentFile, "%s\\%s\0", Path,info.cFileName);
        if(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
              ret = ScanThisDir(CurrentFile,NeedRestore);
		      if(ret == -1)
                 return -1;
        }
        else//it is a file
        {    
           AddToTableOrDelete(CurrentFile,NeedRestore);
        }  
    } 
    FindClose(h); //end of scan  
    return 0;
}

void go()
{
     UCHAR *p; BOOL NeedRestore = FALSE; BOOL SameDir = FALSE;
     sss = NULL;
     nAll=0; nCopy=0; nError=0;
     SMS(hAll,"0"); SMS(hCopy,"0"); SMS(hError,"0");
     
     step = strlen(dropped);
     p=&dropped[step-1];
     if(p[0] =='\\'){ p[0] = 0; } else{ step++; }     
     
     if(ISCHECKED(B_same)) //Same Dir
     {
        SameDir = TRUE;
    //    SNDMSG(B_restore,BM_SETCHECK,(WPARAM)BST_UNCHECKED,(LPARAM)0); 
     }
     else
     {
     
        if(list!=NULL)
           FreeList(); 
        if(ISCHECKED(B_restore))
        {
            NeedRestore = TRUE;
            sprintf(BatCopy, "%s\\restore.bat\0",dropped);     
        }
     }
     ResetEvent(STOP_WORKING);
     ScanThisDir(dropped,NeedRestore);
     SetEvent(STOP_WORKING);
     EnableWindow(B_go,0);
     SMS(B_go,"Start");
     if(SameDir != TRUE)
        FreeList(); 
        
     if(sss) fclose(sss);
     IS_RUNNING = 0;
     SetWindowText(hWnd,"DONE");
}

void LoadInitStuff()
{
   Sleep(500);
   STOP_WORKING = CreateEvent(NULL, TRUE, FALSE, "TTTP");
   if(STOP_WORKING == NULL)
   {
        SetWindowText(hWnd,"#Error: CreateEvent"); Sleep(2000); 
        SNDMSG(hWnd,WM_DESTROY,0,0);
   }
   SetWindowText(hWnd,"Delete Duplicate Files");
}

void CenterOnScreen(HWND hWnd)
{
     RECT rcClient, rcDesktop;
	 int nX, nY;
     SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
     GetWindowRect(hWnd, &rcClient);
	 nX=((rcDesktop.right - rcDesktop.left) / 2) -((rcClient.right - rcClient.left) / 2);
	 nY=((rcDesktop.bottom - rcDesktop.top) / 2) -((rcClient.bottom - rcClient.top) / 2);
SetWindowPos(hWnd, NULL, nX, nY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)                  
  {
     case WM_CREATE:
     {
        HFONT hFont;
        HWND s;
        hWnd = hwnd;  
        hFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Comic Sans MS");
        s=CreateWindow("BUTTON","Total Files:",WSV | BS_GROUPBOX,2,0,140,47,hwnd,NULL,ins,NULL);
        SETFONT(s);
        s=CreateWindow("BUTTON","Copy:",WSV | BS_GROUPBOX,150,0,140,47,hwnd,NULL,ins,NULL);
        SETFONT(s);
        s=CreateWindow("BUTTON","Error:",WSV | BS_GROUPBOX,298,0,140,47,hwnd,NULL,ins,NULL);
        SETFONT(s);
        hFont = CreateFont(28, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "ARIAL");
   
        hAll = CreateWindow("STATIC","",WSV,11,15,122,28,hwnd,NULL,ins,NULL);
        SETFONT(hAll);
        hCopy = CreateWindow("STATIC","",WSV,159,15,122,28,hwnd,NULL,ins,NULL);
        SETFONT(hCopy);
        hError = CreateWindow("STATIC","",WSV,308,15,122,28,hwnd,NULL,ins,NULL);
        SETFONT(hError);
        B_same = CreateWindow("BUTTON","Same DIR",WSV|BS_AUTOCHECKBOX,4,47,90,23,hwnd,(HMENU)1000,ins,NULL);	
        B_go=CreateWindow("BUTTON","Start",WSV|WS_DISABLED,94,47,269,22,hwnd,(HMENU)2222,ins,NULL);
	    B_restore = CreateWindow("BUTTON","Restore",WSV|BS_AUTOCHECKBOX|BS_LEFTTEXT,363,47,74,23,hwnd,(HMENU)1001,ins,NULL);	
                
        CenterOnScreen(hwnd);
        DragAcceptFiles(hwnd,1);
        CreateThread(0,0,(LPTHREAD_START_ROUTINE)LoadInitStuff,0,0,0); 
     }
     break;
     case WM_DROPFILES:
     {
			HDROP hDrop;  DWORD len;			
			memset(dropped,0,MAX_PATH);
			hDrop=(HDROP)wParam;
			DragQueryFile(hDrop,0,dropped,MAX_PATH);
			DragFinish(hDrop);
			if((GetFileAttributes(dropped) & FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
			{  
                MessageBox(hwnd,"Drop a Folder","Info", MB_ICONINFORMATION|MB_OK);
			    break;
			}
            len = strlen(dropped);                              
            if(len < 2)
			{  
                MessageBox(hwnd,"This is a Protected Folder","ERROR", MB_ICONINFORMATION|MB_SYSTEMMODAL|MB_OK);
			    break;
			}
            EnableWindow(B_go,1);
            if(len > 3)
            {
              SetWindowText(hwnd,strrchr(dropped,'\\'));
            }
            else{ SetWindowText(hwnd,dropped); }
            EnableWindow(B_go,1);
     }
	 break;
       case WM_COMMAND:
       {
            switch(LOWORD(wParam))
            {                  
                 case 1000:
                 {
                      if(ISCHECKED(B_same))
                      SNDMSG(B_restore,BM_SETCHECK,(WPARAM)BST_UNCHECKED,(LPARAM)0); 
                 }break;
                 case 1001:
                 {
                      if(ISCHECKED(B_restore))
                      SNDMSG(B_same,BM_SETCHECK,(WPARAM)BST_UNCHECKED,(LPARAM)0); 
                 }break;             
                 
                 case 2222:
                 {
                     if(IS_RUNNING == 0)
                     {
                        CreateThread(0,0,(LPTHREAD_START_ROUTINE)go,0,0,0); 
                        IS_RUNNING = 1;
                        SMS(B_go,"Stop");
                     }
                     else
                     {
                         EnableWindow(B_go,0);
                         SetEvent(STOP_WORKING);
                     }
                 }
                 break;                 
            }
            return 0;
       }
       break;
     case WM_DESTROY:
     {
          CloseHandle(STOP_WORKING);
          PostQuitMessage (0);  
     }
     break;
     default:         
            return DefWindowProc (hwnd, message, wParam, lParam);
   }
    return 0;
}

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil)
{
    MSG messages;    
    WNDCLASSEX wincl;   
    HWND hwnd;
    ins=hThisInstance;

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;  
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hIconSm = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;  
    wincl.cbClsExtra = 0;  
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND+1;

    if (!RegisterClassEx (&wincl))
        return 0;

    hwnd = CreateWindowEx(WS_EX_TOPMOST,szClassName,"....",WS_OVERLAPPED|WS_SYSMENU|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,
    445,102,HWND_DESKTOP,NULL,hThisInstance,NULL );

    while (GetMessage (&messages, NULL, 0, 0))
    {
         TranslateMessage(&messages);
         DispatchMessage(&messages);
    }
     return messages.wParam;
}
