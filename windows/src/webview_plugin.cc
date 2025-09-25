#include "webview_plugin.h"

#include <math.h>
#include <memory>
#include <thread>
#include <iostream>
#include <unordered_map>
#include "include/base/cef_callback.h"

namespace webview_cef
{
	CefMainArgs mainArgs;
	CefRefPtr<WebviewApp> app;
	CefString userAgent;
	bool isCefInitialized = false;

	WebviewPlugin::WebviewPlugin()
	{
		m_handler = new WebviewHandler();
	}

	WebviewPlugin::~WebviewPlugin()
	{
		uninitCallback();
		m_handler->CloseAllBrowsers(true);
		m_handler = nullptr;
		if (!m_renderers.empty())
		{
			m_renderers.clear();
		}
	}

	void WebviewPlugin::initCallback()
	{
		if (!m_init)
		{
			m_handler->onPaintCallback = [=](int browserId, const void *buffer, int32_t width, int32_t height)
			{
				if (m_renderers.find(browserId) != m_renderers.end() && m_renderers[browserId] != nullptr)
				{
					m_renderers[browserId]->onFrame(buffer, width, height);
				}
			};

			m_handler->onTooltipEvent = [=](int browserId, std::string text)
			{
				if (m_invokeFunc)
				{
					WValue *bId = webview_value_new_int(browserId);
					WValue *wText = webview_value_new_string(const_cast<char *>(text.c_str()));
					WValue *retMap = webview_value_new_map();
					webview_value_set_string(retMap, "browserId", bId);
					webview_value_set_string(retMap, "text", wText);
					m_invokeFunc("onTooltip", retMap);
					webview_value_unref(bId);
					webview_value_unref(wText);
					webview_value_unref(retMap);
				}
			};

			m_handler->onCursorChangedEvent = [=](int browserId, int type)
			{
				if (m_invokeFunc)
				{
					WValue *bId = webview_value_new_int(browserId);
					WValue *wType = webview_value_new_int(type);
					WValue *retMap = webview_value_new_map();
					webview_value_set_string(retMap, "browserId", bId);
					webview_value_set_string(retMap, "type", wType);
					m_invokeFunc("onCursorChanged", retMap);
					webview_value_unref(bId);
					webview_value_unref(wType);
					webview_value_unref(retMap);
				}
			};

			m_handler->onUrlChangedEvent = [=](int browserId, std::string url)
			{
				if (m_invokeFunc)
				{
					WValue *bId = webview_value_new_int(browserId);
					WValue *wUrl = webview_value_new_string(const_cast<char *>(url.c_str()));
					WValue *retMap = webview_value_new_map();
					webview_value_set_string(retMap, "browserId", bId);
					webview_value_set_string(retMap, "url", wUrl);
					m_invokeFunc("urlChanged", retMap);
					webview_value_unref(bId);
					webview_value_unref(wUrl);
					webview_value_unref(retMap);
				}
			};

			m_handler->onFocusedNodeChangeMessage = [=](int nBrowserId, bool bEditable)
			{
				if (m_invokeFunc)
				{
					WValue *bId = webview_value_new_int(int64_t(nBrowserId));
					WValue *editable = webview_value_new_bool(bEditable);
					WValue *retMap = webview_value_new_map();
					webview_value_set_string(retMap, "browserId", bId);
					webview_value_set_string(retMap, "editable", editable);
					m_invokeFunc("onFocusedNodeChangeMessage", retMap);
					webview_value_unref(bId);
					webview_value_unref(editable);
					webview_value_unref(retMap);
				}
			};

			m_handler->onLoadStart = [=](int nBrowserId, std::string urlId)
			{
				if (m_invokeFunc)
				{
					WValue *bId = webview_value_new_int(nBrowserId);
					WValue *uId = webview_value_new_string(const_cast<char *>(urlId.c_str()));
					WValue *retMap = webview_value_new_map();
					webview_value_set_string(retMap, "browserId", bId);
					webview_value_set_string(retMap, "urlId", uId);
					m_invokeFunc("onLoadStart", retMap);
					webview_value_unref(bId);
					webview_value_unref(uId);
					webview_value_unref(retMap);
				}
			};

			m_handler->onLoadEnd = [=](int nBrowserId, std::string urlId)
			{
				if (m_invokeFunc)
				{
					WValue *bId = webview_value_new_int(nBrowserId);
					WValue *uId = webview_value_new_string(const_cast<char *>(urlId.c_str()));
					WValue *retMap = webview_value_new_map();
					webview_value_set_string(retMap, "browserId", bId);
					webview_value_set_string(retMap, "urlId", uId);
					m_invokeFunc("onLoadEnd", retMap);
					webview_value_unref(bId);
					webview_value_unref(uId);
					webview_value_unref(retMap);
				}
			};

			m_init = true;
		}
	}

	void WebviewPlugin::uninitCallback()
	{
		m_handler->onPaintCallback = nullptr;
		m_handler->onTooltipEvent = nullptr;
		m_handler->onCursorChangedEvent = nullptr;
		m_handler->onUrlChangedEvent = nullptr;
		m_handler->onFocusedNodeChangeMessage = nullptr;
		m_init = false;
	}

	void WebviewPlugin::HandleMethodCall(std::string name, WValue *values, std::function<void(int, WValue *)> result)
	{
		if (name.compare("init") == 0)
		{
			if (!isCefInitialized)
			{
				if (values != nullptr)
				{
					userAgent = CefString(webview_value_get_string(values));
				}
				startCEF();
			}
			initCallback();
			result(1, nullptr);
		}
		else if (name.compare("quit") == 0)
		{
			// only call this method when you want to quit the app
			//  stopCEF();
			m_handler->CloseAllBrowsers(true);
			result(1, nullptr);
		}
		else if (name.compare("create") == 0)
		{
			std::string url = webview_value_get_string(values);
			m_handler->createBrowser(url, [=](int browserId)
									 {
				std::shared_ptr<WebviewTexture> renderer = m_createTextureFunc();
				m_renderers[browserId] = renderer;
				WValue	*response = webview_value_new_list();
				webview_value_append(response, webview_value_new_int(browserId));
				webview_value_append(response, webview_value_new_int(renderer->textureId));
				result(1, response);
				webview_value_unref(response); });
		}
		else if (name.compare("close") == 0)
		{
			int browserId = int(webview_value_get_int(values));
			m_handler->closeBrowser(browserId);
			if (m_renderers.find(browserId) != m_renderers.end() && m_renderers[browserId] != nullptr)
			{
				m_renderers[browserId].reset();
			}
			result(1, nullptr);
		}
		else if (name.compare("loadUrl") == 0)
		{
			int browserId = int(webview_value_get_int(webview_value_get_list_value(values, 0)));
			const auto url = webview_value_get_string(webview_value_get_list_value(values, 1));
			if (url != nullptr)
			{
				m_handler->loadUrl(browserId, url);
				result(1, nullptr);
			}
		}
		else if (name.compare("setSize") == 0)
		{
			int browserId = int(webview_value_get_int(webview_value_get_list_value(values, 0)));
			const auto dpi = webview_value_get_double(webview_value_get_list_value(values, 1));
			const auto width = webview_value_get_double(webview_value_get_list_value(values, 2));
			const auto height = webview_value_get_double(webview_value_get_list_value(values, 3));
			m_handler->changeSize(browserId, (float)dpi, (int)std::round(width), (int)std::round(height));
			result(1, nullptr);
		}
		else if (name.compare("cursorClickDown") == 0 || name.compare("cursorClickUp") == 0 || name.compare("cursorMove") == 0 || name.compare("cursorDragging") == 0)
		{
			result(cursorAction(values, name), nullptr);
		}
		else if (name.compare("setScrollDelta") == 0)
		{
			int browserId = int(webview_value_get_int(webview_value_get_list_value(values, 0)));
			auto x = webview_value_get_int(webview_value_get_list_value(values, 1));
			auto y = webview_value_get_int(webview_value_get_list_value(values, 2));
			auto deltaX = webview_value_get_int(webview_value_get_list_value(values, 3));
			auto deltaY = webview_value_get_int(webview_value_get_list_value(values, 4));
			m_handler->sendScrollEvent(browserId, (int)x, (int)y, (int)deltaX, (int)deltaY);
			result(1, nullptr);
		}
		else if (name.compare("goForward") == 0)
		{
			int browserId = int(webview_value_get_int(values));
			m_handler->goForward(browserId);
			result(1, nullptr);
		}
		else if (name.compare("goBack") == 0)
		{
			int browserId = int(webview_value_get_int(values));
			m_handler->goBack(browserId);
			result(1, nullptr);
		}
		else if (name.compare("reload") == 0)
		{
			int browserId = int(webview_value_get_int(values));
			m_handler->reload(browserId);
			result(1, nullptr);
		}
		else if (name.compare("openDevTools") == 0)
		{
			int browserId = int(webview_value_get_int(values));
			m_handler->openDevTools(browserId);
			result(1, nullptr);
		}
		else if (name.compare("setClientFocus") == 0)
		{
			int browserId = int(webview_value_get_int(webview_value_get_list_value(values, 0)));
			if (m_renderers.find(browserId) != m_renderers.end() && m_renderers[browserId] != nullptr)
			{
				m_renderers[browserId].get()->isFocused = webview_value_get_bool(webview_value_get_list_value(values, 1));
				m_handler->setClientFocus(browserId, m_renderers[browserId].get()->isFocused);
			}
			result(1, nullptr);
		}
		else
		{
			result = 0;
		}
	}

	void WebviewPlugin::sendKeyEvent(CefKeyEvent &ev)
	{
		m_handler->sendKeyEvent(ev);
		if (ev.type == KEYEVENT_RAWKEYDOWN && ev.windows_key_code == 0x7B && (ev.modifiers & EVENTFLAG_CONTROL_DOWN) != 0)
		{
			for (auto render : m_renderers)
			{
				if (render.second.get()->isFocused)
				{
					m_handler->openDevTools(render.first);
				}
			}
		}
	}

	void WebviewPlugin::setInvokeMethodFunc(std::function<void(std::string, WValue *)> func)
	{
		m_invokeFunc = func;
	}

	void WebviewPlugin::setCreateTextureFunc(std::function<std::shared_ptr<WebviewTexture>()> func)
	{
		m_createTextureFunc = func;
	}

	bool WebviewPlugin::getAnyBrowserFocused()
	{
		for (auto render : m_renderers)
		{
			if (render.second != nullptr && render.second.get()->isFocused)
			{
				return true;
			}
		}
		return false;
	}

	int WebviewPlugin::cursorAction(WValue *args, std::string name)
	{
		if (!args || webview_value_get_len(args) != 3)
		{
			return 0;
		}
		int browserId = int(webview_value_get_int(webview_value_get_list_value(args, 0)));
		int x = int(webview_value_get_int(webview_value_get_list_value(args, 1)));
		int y = int(webview_value_get_int(webview_value_get_list_value(args, 2)));
		if (!x && !y)
		{
			return 0;
		}
		if (name.compare("cursorClickDown") == 0)
		{
			m_handler->cursorClick(browserId, x, y, false);
		}
		else if (name.compare("cursorClickUp") == 0)
		{
			m_handler->cursorClick(browserId, x, y, true);
		}
		else if (name.compare("cursorMove") == 0)
		{
			m_handler->cursorMove(browserId, x, y, false);
		}
		else if (name.compare("cursorDragging") == 0)
		{
			m_handler->cursorMove(browserId, x, y, true);
		}
		return 1;
	}
	int initCEFProcessesWithHInstance(HINSTANCE hInstance)
	{
		// mainArgs = args;
		CefMainArgs args(hInstance);
		mainArgs = args;
		return initCEFProcesses();
	}
	int initCEFProcessesWithArgs(CefMainArgs args)
	{
		mainArgs = args;
		return initCEFProcesses();
	}
	int initCEFProcesses(CefMainArgs args)
	{
		mainArgs = args;
		return initCEFProcesses();
	}

	int initCEFProcesses()
	{
		app = new WebviewApp();
		return CefExecuteProcess(mainArgs, app, nullptr);
	}

	void startCEF()
	{
		CefSettings cefs;
		cefs.windowless_rendering_enabled = true;
		cefs.no_sandbox = true;
		if (!userAgent.empty())
		{
			CefString(&cefs.user_agent_product) = userAgent;
		}
		// locale language setting
		// CefString(&cefs.locale) = "zh-CN";

		// cef message run in another thread on windows/linux
		cefs.multi_threaded_message_loop = true;
		CefInitialize(mainArgs, cefs, app.get(), nullptr);
	}

	void doMessageLoopWork()
	{
		CefDoMessageLoopWork();
	}

	void SwapBufferFromBgraToRgba(void *_dest, const void *_src, int width, int height)
	{
		int32_t *dest = (int32_t *)_dest;
		int32_t *src = (int32_t *)_src;
		int32_t rgba;
		int32_t bgra;
		int length = width * height;
		for (int i = 0; i < length; i++)
		{
			bgra = src[i];
			// BGRA in hex = 0xAARRGGBB.
			rgba = (bgra & 0x00ff0000) >> 16	// Red >> Blue.
				   | (bgra & 0xff00ff00)		// Green Alpha.
				   | (bgra & 0x000000ff) << 16; // Blue >> Red.
			dest[i] = rgba;
		}
	}

	void stopCEF()
	{
		// if(!CefCurrentlyOn(TID_UI)) {
		// 	CefPostTask(TID_UI, base::BindOnce(&CefShutdown));
		// 	return;
		// }
		// CefShutdown();
		// CefPostTask(TID_UI, base::BindOnce(&CefShutdown));
	}
}
