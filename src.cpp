#include <windows.h>
#include <filter.h>

#define TL_ITEM_CUT (WM_USER + 0x46)

FILTER_DLL filter = {
    FILTER_FLAG_ALWAYS_ACTIVE,
    NULL,NULL,
    const_cast<char*>("切り取り"),
    NULL,NULL,NULL,
    NULL,NULL,
    NULL,NULL,NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    func_WndProc,
};
EXTERN_C FILTER_DLL __declspec(dllexport)* __stdcall GetFilterTable(void) {
    return &filter;
}

FILTER* exeditfp;

FILTER* get_exeditfp(FILTER* fp) {
    SYS_INFO si;
    fp->exfunc->get_sys_info(NULL, &si);

    for (int i = 0; i < si.filter_n; i++) {
        FILTER* tfp = (FILTER*)fp->exfunc->get_filterp(i);
        if (tfp->information != NULL) {
            if (!strcmp(tfp->information, "拡張編集(exedit) version 0.92 by ＫＥＮくん")) return tfp;
        }
    }
    return NULL;
}



BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, FILTER* fp) {
    switch (message) {
    case WM_FILTER_INIT:
        exeditfp = get_exeditfp(fp);
        if (exeditfp == NULL) {
            MessageBoxA(fp->hwnd, "拡張編集0.92が見つかりませんでした", fp->name, MB_OK);
            break;
        }
        fp->exfunc->add_menu_item(fp, fp->name, fp->hwnd, TL_ITEM_CUT, 'X', ADD_MENU_ITEM_FLAG_KEY_CTRL);
        break;
    case WM_FILTER_COMMAND:
        if (wparam == TL_ITEM_CUT) {
            SendMessageA(exeditfp->hwnd, WM_COMMAND, 1008, -1);
            SendMessageA(exeditfp->hwnd, WM_COMMAND, 1001, -1);
        }
    }
    return FALSE;
}
