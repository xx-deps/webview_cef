// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include <functional>
#include <list>
#include <unordered_map>

static const char kFocusedNodeChangedMessage[] = "FocusedNodeChanged"; // elements that capture focus in web pages changed message

#define ColorUNDERLINE \
    0xFF000000 // Black SkColor value for underline,
               // same as Blink.
#define ColorBKCOLOR \
    0x00000000 // White SkColor value for background,
               // same as Blink.

struct browser_info
{
    CefRefPtr<CefBrowser> browser;
    uint32_t width = 1;
    uint32_t height = 1;
    float dpi = 1.0;
    bool is_dragging = false;
    CefRect prev_ime_position = CefRect();
    bool is_ime_commit = false;
};

class WebviewHandler : public CefClient,
                       public CefDisplayHandler,
                       public CefLifeSpanHandler,
                       public CefFocusHandler,
                       public CefLoadHandler,
                       public CefRenderHandler
{
public:
    // Paint callback
    std::function<void(int browserId, const void *buffer, int32_t width, int32_t height)> onPaintCallback; // 用于处理浏览器渲染的图像数据
    // cef message event
    std::function<void(int browserId, std::string url)> onUrlChangedEvent;        // 当浏览器的 URL 改变时触发
    std::function<void(int browserId, int type)> onCursorChangedEvent;            // 当浏览器光标改变时触发
    std::function<void(int browserId, std::string text)> onTooltipEvent;          // 当浏览器显示工具提示时触发
    std::function<void(int browserId, bool editable)> onFocusedNodeChangeMessage; // 当浏览器的焦点节点改变时触发
    // webpage message
    std::function<void(int browserId, std::string url)> onLoadStart; // 当浏览器开始加载页面时触发
    std::function<void(int browserId, std::string url)> onLoadEnd;   // 当浏览器完成页面加载时触发

    explicit WebviewHandler();
    ~WebviewHandler();

    // CefClient methods:
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefFocusHandler> GetFocusHandler() override
    {
        return this;
    }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }

    bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override;

    // CefDisplayHandler methods:
    // 浏览器加载的 URL 地址发生变化时被调用。
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 const CefString &url) override;
    //  当浏览器的光标变化时被调用，
    virtual bool OnCursorChange(CefRefPtr<CefBrowser> browser,
                                CefCursorHandle cursor,
                                cef_cursor_type_t type,
                                const CefCursorInfo &custom_cursor_info) override;
    // 当鼠标悬停在元素上时触发，
    virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text) override;
    // 当浏览器的控制台输出消息时被调用
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                  cef_log_severity_t level,
                                  const CefString &message,
                                  const CefString &source,
                                  int line) override;

    // CefLifeSpanHandler methods:
    // 当浏览器窗口创建完成时被调用
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    // 当浏览器关闭时被调用，
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    // 在浏览器关闭之前被调用
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
    // 浏览器准备打开新的弹出窗口时被调用，允许修改弹出窗口的相关设置（例如 URL、窗口信息等）
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               const CefString &target_url,
                               const CefString &target_frame_name,
                               WindowOpenDisposition target_disposition,
                               bool user_gesture,
                               const CefPopupFeatures &popupFeatures,
                               CefWindowInfo &windowInfo,
                               CefRefPtr<CefClient> &client,
                               CefBrowserSettings &settings,
                               CefRefPtr<CefDictionaryValue> &extra_info,
                               bool *no_javascript_access) override;
    // 当浏览器获取焦点时被调用
    virtual void OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) override;
    // 当浏览器的焦点发生变化时被调用，用于处理浏览器的焦点设置
    virtual bool OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) override;
    // 当浏览器获得焦点时被调用。
    virtual void OnGotFocus(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString &errorText,
                             const CefString &failedUrl) override;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefLoadHandler::TransitionType transition_type) override;

    // CefRenderHandler methods:
    // 获取浏览器视图的矩形区域（位置和大小）
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
    // 当浏览器进行绘制时被调用。
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) override;
    // 获取浏览器的屏幕信息，通常用于高 DPI 屏幕支持
    virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo &screen_info) override;
    // 当开始拖动元素时被调用，用于处理拖拽操作。
    virtual bool StartDragging(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefDragData> drag_data,
                               DragOperationsMask allowed_ops,
                               int x,
                               int y) override;

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool force_close);

    // Returns true if the Chrome runtime is enabled.
    static bool IsChromeRuntimeEnabled();


    void closeBrowser(int browserId);     // 关闭指定的浏览器窗口
    void createBrowser(std::string url, std::function<void(int)> callback);  // 创建一个新的浏览器窗口并加载指定的 URL

    void sendScrollEvent(int browserId, int x, int y, int deltaX, int deltaY);// 发送滚动事件，模拟浏览器的滚动操作
    void changeSize(int browserId, float a_dpi, int width, int height); // 改变指定浏览器的大小
    void cursorClick(int browserId, int x, int y, bool up); // 发送鼠标点击事件
    void cursorMove(int browserId, int x, int y, bool dragging); // 发送鼠标移动事件
    void sendKeyEvent(CefKeyEvent &ev); // 发送键盘事件
    void loadUrl(int browserId, std::string url); // 加载指定 URL 的页面
    void goForward(int browserId); // 浏览器前进到下一个页面
    void goBack(int browserId); // 浏览器后退到上一个页面
    void reload(int browserId); // 重新加载页面
    void openDevTools(int browserId); // 打开浏览器的开发者工具

    void setClientFocus(int browserId, bool focus); // 设置浏览器是否获取焦点。

private:
    // List of existing browser windows. Only accessed on the CEF UI thread.
    std::unordered_map<int, browser_info> browser_map_;

    std::unordered_map<std::string, std::function<void(CefRefPtr<CefValue>)>> js_callbacks_;
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(WebviewHandler);
};

#endif // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
