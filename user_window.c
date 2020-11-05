#include "types.h"
#include "stat.h"
//#include "color.h"
#include "msg.h"
#include "user.h"
#include "fcntl.h"
#include "user_window.h"
#include "fs.h"
#include "gui.h"
//#include "character.h"

//#define SCREEN_WIDTH 800
//#define SCREEN_HEIGHT 600
#define CHARACTER_WIDTH 9
#define CHARACTER_HEIGHT 18

void drawColorFillWidget(window *win, Widget *w);
void drawButtonWidget(window *win, Widget *w);
void drawTextWidget(window *win, Widget *w);
void drawInputFieldWidget(window *win, Widget *w);

void debugPrintWidgetList(window *win)
{

    printf(1, "############################\n");
    printf(1, "current Head at %d\n", win->widgetlisthead);
    printf(1, "current Tail at %d\n", win->widgetlisttail);
    printf(1, "current scrollOffset is %d\n", win->scrollOffsetY);
    printf(1, "\n");

    int p;
    for (p = win->widgetlisthead; p != -1; p = win->widgets[p].next)
    {
        printf(1, "current Widget at %d, type: %d, scrollable: %d\n", p, win->widgets[p].type, win->widgets[p].scrollable);
        printf(1, "current Widget position %d, %d, %d, %d\n", win->widgets[p].position.xmin, win->widgets[p].position.ymin, win->widgets[p].position.xmax, win->widgets[p].position.ymax);
        printf(1, "prev Window at %d\n", win->widgets[p].prev);
        printf(1, "next Window at %d\n", win->widgets[p].next);
        //cprintf("current Window width %d\n", windowlist[p].wnd->position.xmax - windowlist[p].wnd->position.xmin);
        printf(1, "\n");
    }
}

int isInRect(int xmin, int ymin, int xmax, int ymax, int x, int y)
{
    return (x >= xmin && x <= xmax && y >= ymin && y <= ymax);
}

void createPopupWindow(window *win, int caller)
{

    int width = win->width;
    int height = win->height;

    win->window_buf = malloc(width * height * 3);
    if (!win->window_buf)
    {
        return;
    }
    memset(win->window_buf, 255, height * width * 3);
    win->widgetlisthead = -1;
    win->widgetlisttail = -1;
    int i;
    for (i = 0; i < MAX_WIDGET_SIZE; ++i)
    {
        win->widgets[i].next = i;
        win->widgets[i].prev = i;
    }
    win->needsRepaint = 1;
    win->hasTitleBar = 0;
    win->scrollOffsetX = 0;
    win->scrollOffsetY = 0;
    //initlock(&win->wmlock, "wmlock");
    GUI_createPopupWindow(win, caller);
}

void closePopupWindow(window *win)
{
    free(win->window_buf);
    GUI_closePopupWindow(win);
    exit();
}

void createWindow(window *win, const char *title)
{

    int width = win->width;
    int height = win->height;

    win->window_buf = malloc(width * height * 3);
    if (!win->window_buf)
    {
        return;
    }
    memset(win->window_buf, 255, height * width * 3);
    win->widgetlisthead = -1;
    win->widgetlisttail = -1;
    win->keyfocus = -1;
    win->scrollOffsetX = 0;
    win->scrollOffsetY = 0;
    int i;
    for (i = 0; i < MAX_WIDGET_SIZE; ++i)
    {
        win->widgets[i].next = i;
        win->widgets[i].prev = i;
    }
    win->needsRepaint = 1;
    if (win->hasTitleBar != 0)
    {
        win->hasTitleBar = 1;
    }
    
    GUI_createWindow(win, title);
}

void closeWindow(window *win)
{
    free(win->window_buf);
    GUI_closeWindow(win);
    exit();
}

void updateWindow(window *win)
{
    //debugPrintWidgetList(win);
    if (win->needsRepaint)
    {
        //memset(win->window_buf, 255, win->height * win->width * 3);
        for (int p = win->widgetlisthead; p != -1; p = win->widgets[p].next)
        {
            //don't draw widget that is invisible
            
            if ((!win->widgets[p].scrollable && (win->widgets[p].position.xmin > win->width ||
                                                 win->widgets[p].position.xmax < 0 ||
                                                 win->widgets[p].position.ymin > win->height ||
                                                 win->widgets[p].position.ymax < 0)) ||
                (win->widgets[p].scrollable && (win->widgets[p].position.xmin - win->scrollOffsetX > win->width ||
                                                win->widgets[p].position.xmax - win->scrollOffsetX < 0 ||
                                                win->widgets[p].position.ymin - win->scrollOffsetY > win->height ||
                                                win->widgets[p].position.ymax - win->scrollOffsetY < 0)))
            {
                continue;
            }

            switch (win->widgets[p].type)
            {
            case COLORFILL:
                drawColorFillWidget(win, &win->widgets[p]);
                break;
            case BUTTON:
                drawButtonWidget(win, &win->widgets[p]);
                break;
            case TEXT:
                drawTextWidget(win, &win->widgets[p]);
                break;
            case INPUTFIELD:
                drawInputFieldWidget(win, &win->widgets[p]);
                break;

            default:
                break;
            }
        }
    }

    message msg;
    if (GUI_getMessage(win->handler, &msg) == 0)
    {
        win->needsRepaint = 1;
        if (msg.msg_type == WM_WINDOW_CLOSE)
        {
            closeWindow(win);
        }
        else if (msg.msg_type == WM_WINDOW_MINIMIZE)
        {
            GUI_minimizeWindow(win);
        }
        else if (msg.msg_type == WM_WINDOW_MAXIMIZE)
        {
            GUI_maximizeWindow(win);
        }
        else
        {
            if (msg.msg_type == M_KEY_DOWN || msg.msg_type == M_KEY_UP)
            {
                win->widgets[win->keyfocus].handler(&win->widgets[win->keyfocus], &msg);
            }
            else
            {
                int mouse_x = msg.params[0];
                int mouse_y = msg.params[1];

                for (int p = win->widgetlisttail; p != -1; p = win->widgets[p].prev)
                {

                    if ((!win->widgets[p].scrollable && isInRect(win->widgets[p].position.xmin, win->widgets[p].position.ymin, win->widgets[p].position.xmax, win->widgets[p].position.ymax, mouse_x, mouse_y)) ||
                        (win->widgets[p].scrollable && isInRect(win->widgets[p].position.xmin - win->scrollOffsetX, win->widgets[p].position.ymin - win->scrollOffsetY, win->widgets[p].position.xmax - win->scrollOffsetX, win->widgets[p].position.ymax - win->scrollOffsetY, mouse_x, mouse_y)))
                    {
                        if (!win->widgets[p].scrollable)
                        {
                            win->widgets[p].handler(&win->widgets[p], &msg);
                        }
                        else
                        {
                            message newmsg;
                            newmsg.msg_type = msg.msg_type;
                            newmsg.params[0] = msg.params[0] + win->scrollOffsetX;
                            newmsg.params[1] = msg.params[1] + win->scrollOffsetY;
                            win->widgets[p].handler(&win->widgets[p], &newmsg);
                        }

                        if(win->widgets[p].type==INPUTFIELD) {
                            win->keyfocus=p;
                        }

                        break;
                    }
                }
            }
        }
    }
    else
    {
        win->needsRepaint = 0;
    }
    return;
}

void updatePopupWindow(window *win)
{
    if (win->needsRepaint)
    {
        for (int p = win->widgetlisthead; p != -1; p = win->widgets[p].next)
        {
            drawButtonWidget(win, &win->widgets[p]);
        }
    }

    message msg;
    if (GUI_getPopupMessage(&msg) == 0)
    {
        win->needsRepaint = 1;
        if (msg.msg_type != 0)
        {
            printf(1, "message is %d\n", msg.msg_type);
            printf(1, "mouse at %d, %d\n", msg.params[0], msg.params[1]);
        }
        if (msg.msg_type == WM_WINDOW_CLOSE)
        {
            closePopupWindow(win);
        }
        else
        {
            if (msg.msg_type == M_KEY_DOWN || msg.msg_type == M_KEY_UP)
            {
                win->widgets[win->keyfocus].handler(&win->widgets[win->keyfocus], &msg);
            }
            else
            {
                int mouse_x = msg.params[0];
                int mouse_y = msg.params[1];
                for (int p = win->widgetlisttail; p != -1; p = win->widgets[p].prev)
                {
                    if (isInRect(win->widgets[p].position.xmin, win->widgets[p].position.ymin, win->widgets[p].position.xmax, win->widgets[p].position.ymax, mouse_x, mouse_y))
                    {
                        win->widgets[p].handler(&win->widgets[p], &msg);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        win->needsRepaint = 0;
    }
    return;
}

void setWidgetSize(Widget *widget, int x, int y, int w, int h)
{
    widget->position.xmin = x;
    widget->position.ymin = y;
    widget->position.xmax = x + w;
    widget->position.ymax = y + h;
}

int findNextAvailable(window *win)
{

    for (int i = 0; i < MAX_WIDGET_SIZE; i++)
    {
        if (win->widgets[i].prev == i && win->widgets[i].next == i)
        {
            return i;
        }
    }
    return -1;
}

int findWidgetId(window *win, Widget *widget)
{

    for (int i = 0; i < MAX_WIDGET_SIZE; i++)
    {
        if (&win->widgets[i] == widget)
        {
            return i;
        }
    }
    return -1;
}

void addToWidgetListTail(window *win, int idx)
{
    win->widgets[idx].prev = win->widgetlisttail;
    win->widgets[idx].next = -1;
    if (win->widgetlisttail != -1)
        win->widgets[win->widgetlisttail].next = idx;
    win->widgetlisttail = idx;
}

void removeFromWidgetList(window *win, int idx)
{
    if (win->widgetlisthead == idx)
        win->widgetlisthead = win->widgets[win->widgetlisttail].next;
    if (win->widgetlisttail == idx)
        win->widgetlisttail = win->widgets[win->widgetlisttail].prev;
    if (win->widgets[idx].prev != -1)
        win->widgets[win->widgets[idx].prev].next = win->widgets[idx].next;
    if (win->widgets[idx].next != -1)
        win->widgets[win->widgets[idx].next].prev = win->widgets[idx].prev;
    win->widgets[idx].prev = idx;
    win->widgets[idx].next = idx;
}

int addWidget(window *win)
{
    int widgetId = findNextAvailable(win);
    if (widgetId == -1)
        return -1;

    if (win->widgetlisthead == -1)
    {
        win->widgetlisthead = widgetId;
    }

    addToWidgetListTail(win, widgetId);
    return widgetId;
}

int removeWidget(window *win, int index)
{
    if (win->widgets[index].prev == index && win->widgets[index].next == index)
    {
        return -1;
    }
    switch (win->widgets[index].type)
    {
    case COLORFILL:
        free(win->widgets[index].context.colorfill);
        break;
    case BUTTON:
        free(win->widgets[index].context.button);
        break;
    case TEXT:
        free(win->widgets[index].context.text);
        break;
    case INPUTFIELD:
        free(win->widgets[index].context.inputfield);
        break;

    default:
        break;
    }
    removeFromWidgetList(win, index);
    return 0;
}

int setWidgetHandler(window *win, int index, Handler handler)
{
    win->widgets[index].handler = handler;
    return 0;
}

int addColorFillWidget(window *win, RGBA c, int x, int y, int w, int h, int scrollable, Handler handler)
{

    int widgetId = addWidget(win);
    if (widgetId == -1)
        return -1;
    ColorFill *b = malloc(sizeof(ColorFill));
    b->buf = malloc(w * h * 3);
    fillRect(b->buf, 0, 0, w, h, w, h, c);

    Widget *widget = &win->widgets[widgetId];
    widget->context.colorfill = b;
    widget->type = COLORFILL;
    widget->handler = handler;
    widget->scrollable = scrollable;
    //widget->window=win;
    setWidgetSize(widget, x, y, w, h);

    return widgetId;
}

int addButtonWidget(window *win, RGBA c, RGBA bc, char *text, int x, int y, int w, int h, int scrollable, Handler handler)
{

    int widgetId = addWidget(win);
    if (widgetId == -1)
        return -1;
    Button *b = malloc(sizeof(Button));
    b->bg_color = bc;
    b->color = c;
    strcpy(b->text, text);

    Widget *widget = &win->widgets[widgetId];
    widget->context.button = b;
    widget->type = BUTTON;
    widget->handler = handler;
    widget->scrollable = scrollable;
    setWidgetSize(widget, x, y, w, h);

    return widgetId;
}

int addTextWidget(window *win, RGBA c, char *text, int x, int y, int w, int h, int scrollable, Handler handler)
{

    int widgetId = addWidget(win);
    if (widgetId == -1)
        return -1;
    Text *t = malloc(sizeof(Text));
    t->color = c;
    strcpy(t->text, text);

    Widget *widget = &win->widgets[widgetId];
    widget->context.text = t;
    widget->type = TEXT;
    widget->handler = handler;
    widget->scrollable = scrollable;
    setWidgetSize(widget, x, y, w, h);

    return widgetId;
}

int addInputFieldWidget(window *win, RGBA c, char *text, int x, int y, int w, int h, int scrollable, Handler handler)
{

    int widgetId = addWidget(win);
    if (widgetId == -1)
        return -1;
    InputField *t = malloc(sizeof(InputField));
    t->color = c;
    strcpy(t->text, text);
    t->current_pos = strlen(text);

    Widget *widget = &win->widgets[widgetId];
    widget->context.inputfield = t;
    widget->type = INPUTFIELD;
    widget->handler = handler;
    widget->scrollable = scrollable;
    setWidgetSize(widget, x, y, w, h);

    win->keyfocus = widgetId;

    return widgetId;
}

void drawColorFillWidget(window *win, Widget *w)
{
    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;
    //window_drawFillRect(win, w->context.colorfill->color, w->position.xmin, w->position.ymin, width, height);
    if (w->scrollable)
    {
        draw24Image(win, w->context.colorfill->buf, w->position.xmin - win->scrollOffsetX, w->position.ymin - win->scrollOffsetY, width, height);
    }
    else
    {
        draw24Image(win, w->context.colorfill->buf, w->position.xmin, w->position.ymin, width, height);
    }
}

void drawButtonWidget(window *win, Widget *w)
{
    RGB black;
    black.R = 0;
    black.G = 0;
    black.B = 0;
    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;
    int textYOffset = (height - CHARACTER_HEIGHT) / 2;
    int textXOffset = 2;
    if (width > strlen(w->context.button->text) * CHARACTER_WIDTH)
    {
        textXOffset = (width - strlen(w->context.button->text) * CHARACTER_WIDTH) / 2;
    }
    if (w->scrollable)
    {
        drawFillRect(win, w->context.button->bg_color, w->position.xmin - win->scrollOffsetX, w->position.ymin - win->scrollOffsetY, width, height);
        drawRect(win, black, w->position.xmin - win->scrollOffsetX, w->position.ymin - win->scrollOffsetY, width, height);
        drawString(win, w->context.button->text, w->context.button->color, w->position.xmin + 4 - win->scrollOffsetX, w->position.ymin + 2 - win->scrollOffsetY, width, height);
    }
    else
    {
        drawFillRect(win, w->context.button->bg_color, w->position.xmin, w->position.ymin, width, height);
        drawRect(win, black, w->position.xmin, w->position.ymin, width, height);
        drawString(win, w->context.button->text, w->context.button->color, w->position.xmin + textXOffset, w->position.ymin + textYOffset, width, height);
    }
    //drawString(win, w->position.xmin+4, w->position.ymin+2, w->context.button->text, w->context.button->color, width);
}

void drawTextWidget(window *win, Widget *w)
{
    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;
    if (w->scrollable)
    {
        drawString(win, w->context.text->text, w->context.text->color, w->position.xmin - win->scrollOffsetX, w->position.ymin - win->scrollOffsetY, width, height);
    }
    else
    {
        drawString(win, w->context.text->text, w->context.text->color, w->position.xmin, w->position.ymin, width, height);
    }
    //drawString(win, w->context.text->text, w->context.text->color, w->position.xmin, w->position.ymin, width, height);
}

void drawInputFieldWidget(window *win, Widget *w)
{
    int width = w->position.xmax - w->position.xmin;
    int height = w->position.ymax - w->position.ymin;
    //int charPerLine = width / CHARACTER_WIDTH;
    //int charCount = strlen(w->context.inputfield->text);

    int offset_x = 0;
    int offset_y = 0;
    //int charPerLine = width / CHARACTER_WIDTH;
    if (w->scrollable)
    {
        drawString(win, w->context.inputfield->text, w->context.inputfield->color, w->position.xmin - win->scrollOffsetX, w->position.ymin - win->scrollOffsetY, width, height);
    }
    else
    {
        drawString(win, w->context.inputfield->text, w->context.inputfield->color, w->position.xmin, w->position.ymin, width, height);
    }

    int iter = 0;
    while (iter < w->context.inputfield->current_pos)
    {
        if (offset_y > height)
            break;
        if (w->context.inputfield->text[iter] != '\n')
        {
            offset_x += CHARACTER_WIDTH;
            if (offset_x > width)
            {
                offset_x = 0;
                offset_y += CHARACTER_HEIGHT;
            }
        }
        else
        {
            offset_x = 0;
            offset_y += CHARACTER_HEIGHT;
        }
        iter++;
    }
    RGBA black;
    black.R = 0;
    black.G = 0;
    black.B = 0;
    black.A = 255;
    //printf(1, "cursor at %d, %d\n", offset_x, offset_y);
    if (offset_y < height)
    {
        if (w->scrollable)
        {
            drawFillRect(win, black, w->position.xmin + offset_x - win->scrollOffsetX, w->position.ymin + offset_y + 1 - win->scrollOffsetY, 1, CHARACTER_HEIGHT - 4);
        }
        else
        {
            drawFillRect(win, black, w->position.xmin + offset_x, w->position.ymin + offset_y + 1, 1, CHARACTER_HEIGHT - 4);
        }
    }
}