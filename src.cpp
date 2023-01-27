#include <windows.h>
#include <filter.h>
#include <exedit.hpp>

#define TL_ITEM_CUT (WM_USER + 0x46)
#define TL_ITEM_CUT_RIPPLE (WM_USER + 0x47)

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
int** exeditbuf_ptr; // 0x1a5328
int* ObjectAlloc_ptr; // 0x1e0fa0
int* SortedObjectLayerBeginIndex; // 0x149670
int* SortedObjectLayerEndIndex; // 0x135ac8
ExEdit::Object** SortedObject; // 0x168fa8
ExEdit::Object** ObjectArray_ptr; // 0x1e0fa4
int* SelectingObjectNum_ptr; // 0x167d88
int* SelectingObjectIndex; // 0x179230
int* SettingDialogObjectIndex; // 0x177a10
int* NextObjectIndex; // 0x1592d8
int* undo_id_ptr; // 0x244e14

static inline void(__cdecl* nextundo)();
static inline void(__cdecl* setundo)(int objidx, int flag);
static inline void(__cdecl* delobj)(int objidx); // 34500
static inline void(__cdecl* drawtimeline)(); // 39230

void cut() {
    SendMessageA(exeditfp->hwnd, WM_COMMAND, 1008, -1);
    SendMessageA(exeditfp->hwnd, WM_COMMAND, 1001, -1);
}


void ripple_cut() {
    auto obj = *ObjectArray_ptr;
    int n = *SelectingObjectNum_ptr;
    if (0 < n) {
        int* exeditbuf = *exeditbuf_ptr;
        int objectalloc = *ObjectAlloc_ptr;
        memset(exeditbuf, 0, objectalloc * 4);
        SendMessageA(exeditfp->hwnd, WM_COMMAND, 1008, -1);
        for (int i = 0; i < n; i++) {
            auto selectobj = obj + SelectingObjectIndex[i];
            int objidx = ((int)selectobj - (int)obj) / sizeof(ExEdit::Object);
            if ((*(int*)&selectobj->flag & 1) && 0 <= exeditbuf[objidx]) {
                int frame = 0;
                int ledidx = selectobj->index_midpt_leader;
                if (ledidx == -1) {
                    exeditbuf[objidx] = -1;
                    frame += selectobj->frame_end - selectobj->frame_begin + 1;
                } else {
                    objidx = ledidx;
                    while (0 <= objidx) {
                        exeditbuf[objidx] = -1;
                        frame += obj[objidx].frame_end - obj[objidx].frame_begin + 1;
                        objidx = NextObjectIndex[objidx];
                    }
                }
                int layer = selectobj->layer_set;
                for (int j = SortedObjectLayerBeginIndex[layer]; j <= SortedObjectLayerEndIndex[layer]; j++) {
                    if (SortedObject[j] == selectobj) {
                        for (j++; j <= SortedObjectLayerEndIndex[layer]; j++) {
                            objidx = ((int)SortedObject[j] - (int)obj) / sizeof(ExEdit::Object);
                            if (0 <= exeditbuf[objidx]) {
                                exeditbuf[objidx] += frame;
                            }
                        }
                    }
                }
            }
        }

        nextundo();
        for (int i = 0; i < n; i++) {
            auto selectobj = obj + SelectingObjectIndex[i];
            if (*(int*)&selectobj->flag & 1) {
                int objidx = ((int)selectobj - (int)obj) / sizeof(ExEdit::Object);
                delobj(objidx);
            }
        }
        *SelectingObjectNum_ptr = 0;
        int flag = 1;
        for (int i = 0; i < objectalloc; i++) {
            if ((*(int*)&obj[i].flag & 1) && 0 < exeditbuf[i]) {
                if (flag) {
                    flag = 0;
                }
                setundo(i, 8);
                obj[i].frame_begin -= exeditbuf[i];
                obj[i].frame_end -= exeditbuf[i];
            }
        }
        drawtimeline();
    } else if (0 <= *SettingDialogObjectIndex) {
        int objidx = *SettingDialogObjectIndex;
        int ledidx = obj[objidx].index_midpt_leader;
        if (ledidx == -1) {
            SelectingObjectIndex[0] = objidx;
            *SelectingObjectNum_ptr = 1;
        } else {
            int i = 0;
            objidx = ledidx;
            while (0 <= objidx) {
                SelectingObjectIndex[i] = objidx;
                i++;
                objidx = NextObjectIndex[objidx];
            }
            *SelectingObjectNum_ptr = i;
        }
        ripple_cut();
    }
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
        fp->exfunc->add_menu_item(fp, const_cast<char*>("リップル"), fp->hwnd, TL_ITEM_CUT_RIPPLE, 'X', ADD_MENU_ITEM_FLAG_KEY_CTRL | ADD_MENU_ITEM_FLAG_KEY_SHIFT);
        
        exeditbuf_ptr = (int**)((int)exeditfp->dll_hinst + 0x1a5328);
        ObjectAlloc_ptr = (int*)((int)exeditfp->dll_hinst + 0x1e0fa0);
        SortedObjectLayerBeginIndex = (int*)((int)exeditfp->dll_hinst + 0x149670);
        SortedObjectLayerEndIndex = (int*)((int)exeditfp->dll_hinst + 0x135ac8);
        SortedObject = (ExEdit::Object**)((int)exeditfp->dll_hinst + 0x168fa8);
        ObjectArray_ptr = (ExEdit::Object**)((int)exeditfp->dll_hinst + 0x1e0fa4);
        SelectingObjectNum_ptr = (int*)((int)exeditfp->dll_hinst + 0x167d88);
        SelectingObjectIndex = (int*)((int)exeditfp->dll_hinst + 0x179230);
        SettingDialogObjectIndex = (int*)((int)exeditfp->dll_hinst + 0x177a10);
        NextObjectIndex = (int*)((int)exeditfp->dll_hinst + 0x1592d8);
        undo_id_ptr = (int*)((int)exeditfp->dll_hinst + 0x244e14);
        nextundo = reinterpret_cast<decltype(nextundo)>((int)exeditfp->dll_hinst + 0x8d150);
        setundo = reinterpret_cast<decltype(setundo)>((int)exeditfp->dll_hinst + 0x8d290);
        delobj = reinterpret_cast<decltype(delobj)>((int)exeditfp->dll_hinst + 0x34500);
        drawtimeline = reinterpret_cast<decltype(drawtimeline)>((int)exeditfp->dll_hinst + 0x39230);

        break;
    case WM_FILTER_COMMAND:
        switch (wparam) {
        case TL_ITEM_CUT:
            cut();
            break;
        case TL_ITEM_CUT_RIPPLE:
            ripple_cut();
        }
    }
    return FALSE;
}
