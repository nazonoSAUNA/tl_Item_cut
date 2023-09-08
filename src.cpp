#include <windows.h>
#include <filter.h>
#include <exedit.hpp>

#define TL_ITEM_CUT (WM_USER + 0x46)
#define TL_ITEM_CUT_RIPPLE (WM_USER + 0x47)
#define TL_ITEM_PREV_GAP_DEL (WM_USER + 0x48)
#define TL_CUT_RIPPLE (WM_USER + 0x5e)

#define TL_ITEM_GROUP_SPLIT (WM_USER + 0x1b)
#define TL_ITEM_GROUP_RIPPLE (WM_USER + 0x1c)

static const char tl_ripple[] = ("TLリップル");
static const char prev_gap_del[] = ("前ギャップ削除");
static const char group_split[] = ("グループ分割");
static const char group_ripple[] = ("グループリップル");

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
int* split_mode; // 0x1538b0

static inline void(__cdecl* nextundo)();
static inline void(__cdecl* setundo)(int objidx, int flag);
static inline void(__cdecl* delobj)(int objidx); // 34500
static inline void(__cdecl* drawtimeline)(); // 39230
static inline void(__cdecl* update_setting_dialog_top)(); // 2c580
static inline void(__cdecl* disp_near_setting_dialog)(); // 446d0

void sort_select_obj() {
    int n = *SelectingObjectNum_ptr - 1;
    if (n < 1) return;

    auto obj = *ObjectArray_ptr;
    int sort_begin = 0;
    int sort_end = n;
    while (true) {
        int lastswap = 0;
        int i = sort_begin;
        int j = i + n;
        while (j <= sort_end) {
            if (obj[SelectingObjectIndex[i]].frame_begin > obj[SelectingObjectIndex[j]].frame_begin) {
                std::swap(SelectingObjectIndex[i], SelectingObjectIndex[j]);
                lastswap = i;
            }
            i++; j++;
        }
        switch (n) {
        case 12: case 13: case 14: case 15:
            n = 11;
            break;
        case 1:
            if (lastswap == 0) return;
            sort_end = lastswap;
            break;
        default:
            n = n * 10 / 13;
        }
    }

    for (int i = 0; i < n; i++) {
        auto selectobj = obj + SelectingObjectIndex[i];

    }
}

void select_dialog_obj() {
    int objidx = *SettingDialogObjectIndex;
    auto obj = *ObjectArray_ptr;
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
}

void select_dialog_group_obj() {
    if (*SettingDialogObjectIndex < 0) return;

    auto obj = *ObjectArray_ptr + *SettingDialogObjectIndex;
    if ((int)obj->flag == 0) return;

    int group_belong = obj->group_belong;
    if (group_belong == 0) {
        group_belong = -1;
    }
    int index_midpt_leader = obj->index_midpt_leader;
    if (index_midpt_leader == -1) {
        index_midpt_leader = -2;
    }
    obj = *ObjectArray_ptr;

    int n = 1;
    SelectingObjectIndex[0] = *SettingDialogObjectIndex;
    for (int i = 0; i < *ObjectAlloc_ptr; i++) {
        if (((int)obj->flag & 1) && obj->layer_disp != -1 && (obj->group_belong == group_belong || obj->index_midpt_leader == index_midpt_leader) && i != *SettingDialogObjectIndex) {
            SelectingObjectIndex[n] = i;
            n++;
        }
        obj++;
    }
    *SelectingObjectNum_ptr = n;
}


void item_cut() {
    SendMessageA(exeditfp->hwnd, WM_COMMAND, 1008, -1);
    SendMessageA(exeditfp->hwnd, WM_COMMAND, 1001, -1);
}


void item_ripple_cut() {
    int n = *SelectingObjectNum_ptr;
    if (0 < n) {
        auto obj = *ObjectArray_ptr;
        int* exeditbuf = *exeditbuf_ptr;
        int objectalloc = *ObjectAlloc_ptr;
        memset(exeditbuf, 0, objectalloc * 4);
        SendMessageA(exeditfp->hwnd, WM_COMMAND, 1008, -1);
        for (int i = 0; i < n; i++) {
            auto selectobj = obj + SelectingObjectIndex[i];
            int objidx = ((int)selectobj - (int)obj) / sizeof(ExEdit::Object);
            if (((int)selectobj->flag & 1) && 0 <= exeditbuf[objidx]) {
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

        disp_near_setting_dialog();
        nextundo();
        for (int i = 0; i < n; i++) {
            auto selectobj = obj + SelectingObjectIndex[i];
            if ((int)selectobj->flag & 1) {
                int objidx = ((int)selectobj - (int)obj) / sizeof(ExEdit::Object);
                delobj(objidx);
            }
        }
        *SelectingObjectNum_ptr = 0;
        for (int i = 0; i < objectalloc; i++) {
            if (((int)obj[i].flag & 1) && 0 < exeditbuf[i]) {
                setundo(i, 8);
                obj[i].frame_begin -= exeditbuf[i];
                obj[i].frame_end -= exeditbuf[i];
            }
        }
        drawtimeline();
    } else if (0 <= *SettingDialogObjectIndex) {
        select_dialog_obj();
        item_ripple_cut();
    }
}

void select_all() {
    auto obj = *ObjectArray_ptr;
    int objectalloc = *ObjectAlloc_ptr;
    int n = 0;
    for (int i = 0; i < objectalloc; i++) {
        if (((int)obj->flag & 1) && obj->layer_disp != -1) {
            SelectingObjectIndex[n] = i;
            n++;
        }
        obj++;
    }
    *SelectingObjectNum_ptr = n;
}

void tl_ripple_cut(void* editp, FILTER* fp) {
    int n = *SelectingObjectNum_ptr;
    if (0 < n) {
        auto obj = *ObjectArray_ptr;
        int frame_begin = INT_MAX;
        int frame_end = -1;
        for (int i = 0; i < n; i++) {
            auto selectobj = obj + SelectingObjectIndex[i];
            if (selectobj->frame_begin < frame_begin) {
                frame_begin = selectobj->frame_begin;
            }
            if (frame_end < selectobj->frame_end) {
                frame_end = selectobj->frame_end;
            }
        }
        if (frame_begin < frame_end) {
            fp->exfunc->set_select_frame(editp, frame_begin, frame_end);
            select_all();
            SendMessageA(exeditfp->hwnd, WM_COMMAND, 1094, -1);
        }
    } else if (0 <= *SettingDialogObjectIndex) {
        select_dialog_obj();
        tl_ripple_cut(editp, fp);
    }
}

void item_prev_gap_del() {
    int n = *SelectingObjectNum_ptr;
    if (0 < n) {
        sort_select_obj();
        auto obj = *ObjectArray_ptr;
        int flag = 1;
        for (int i = 0; i < n; i++) {
            auto selectobj = obj + SelectingObjectIndex[i];
            if (((int)selectobj->flag & 1)) {
                int layer = selectobj->layer_set;
                int begin_idx = SortedObjectLayerBeginIndex[layer];
                for (int j = begin_idx; j <= SortedObjectLayerEndIndex[layer]; j++) {
                    if (SortedObject[j] == selectobj) {
                        int gap;
                        if (begin_idx == j) {
                            gap = selectobj->frame_begin;
                        } else {
                            gap = selectobj->frame_begin - SortedObject[j - 1]->frame_end - 1;
                        }
                        if (0 < gap) {
                            if (flag) {
                                flag = 0;
                                nextundo();
                            }
                            setundo(SelectingObjectIndex[i], 8);
                            selectobj->frame_begin -= gap;
                            selectobj->frame_end -= gap;
                        }
                        break;
                    }
                }
            }
        }
        drawtimeline();
        update_setting_dialog_top();
    } else if (0 <= *SettingDialogObjectIndex) {
        select_dialog_obj();
        item_prev_gap_del();
    }
}

void item_group_split() {
    select_dialog_group_obj();
    if (*SelectingObjectNum_ptr <= 0) return;
    
    int split_mode_org = *split_mode;
    *split_mode = 1;
    SendMessageA(exeditfp->hwnd, WM_COMMAND, 1051, -1);
    *split_mode = split_mode_org;

    *SelectingObjectNum_ptr = 0;
    drawtimeline();
}
void item_group_ripple() {
    select_dialog_group_obj();
    if (*SelectingObjectNum_ptr <= 0) return;

    item_ripple_cut();
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
        fp->exfunc->add_menu_item(fp, (LPSTR)&tl_ripple[2], fp->hwnd, TL_ITEM_CUT_RIPPLE, 'X', ADD_MENU_ITEM_FLAG_KEY_CTRL | ADD_MENU_ITEM_FLAG_KEY_SHIFT);
        fp->exfunc->add_menu_item(fp, (LPSTR)tl_ripple, fp->hwnd, TL_CUT_RIPPLE, 'X', ADD_MENU_ITEM_FLAG_KEY_CTRL | ADD_MENU_ITEM_FLAG_KEY_ALT);
        fp->exfunc->add_menu_item(fp, (LPSTR)prev_gap_del, fp->hwnd, TL_ITEM_PREV_GAP_DEL, NULL, NULL);
        fp->exfunc->add_menu_item(fp, (LPSTR)group_split, fp->hwnd, TL_ITEM_GROUP_SPLIT, NULL, NULL);
        fp->exfunc->add_menu_item(fp, (LPSTR)group_ripple, fp->hwnd, TL_ITEM_GROUP_RIPPLE, NULL, NULL);

        
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
        split_mode = (int*)((int)exeditfp->dll_hinst + 0x1538b0);
        nextundo = reinterpret_cast<decltype(nextundo)>((int)exeditfp->dll_hinst + 0x8d150);
        setundo = reinterpret_cast<decltype(setundo)>((int)exeditfp->dll_hinst + 0x8d290);
        delobj = reinterpret_cast<decltype(delobj)>((int)exeditfp->dll_hinst + 0x34500);
        drawtimeline = reinterpret_cast<decltype(drawtimeline)>((int)exeditfp->dll_hinst + 0x39230);
        update_setting_dialog_top = reinterpret_cast<decltype(update_setting_dialog_top)>((int)exeditfp->dll_hinst + 0x2c580);
        disp_near_setting_dialog = reinterpret_cast<decltype(disp_near_setting_dialog)>((int)exeditfp->dll_hinst + 0x446d0);


        break;
    case WM_FILTER_COMMAND:
        switch (wparam) {
        case TL_ITEM_CUT:
            item_cut();
            break;
        case TL_ITEM_CUT_RIPPLE:
            item_ripple_cut();
            break;
        case TL_CUT_RIPPLE:
            tl_ripple_cut(editp, fp);
            break;
        case TL_ITEM_PREV_GAP_DEL:
            item_prev_gap_del();
            break;
        case TL_ITEM_GROUP_SPLIT:
            item_group_split();
            break;
        case TL_ITEM_GROUP_RIPPLE:
            item_group_ripple();
        }
    }
    return FALSE;
}
