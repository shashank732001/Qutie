#ifndef EDGEVIEW_H
#define EDGEVIEW_H
#pragma once

#include <windows.h>
#include <wrl.h>
#include <functional>
#include <string>
#include "WebView2.h"

class EdgeView {
public:
    EdgeView();
    ~EdgeView();

    HRESULT Initialize(HWND parent);
    HRESULT Initialize(HWND parent, const RECT& bounds);
    HRESULT Navigate(const wchar_t* url) const;
    HRESULT SetBounds(const RECT& bounds);
    void    ResizeToClient();
    HRESULT SetVisible(BOOL visible);
    void SetReadyCallback(std::function<void()> cb);
    bool IsReady() const { return m_controller != nullptr; }
    HWND Parent()  const { return m_parent; }

private:
    void OnReady();
    HRESULT EnsureUserDataFolder();

    HWND m_parent = nullptr;
    RECT m_bounds { 0, 0, 0, 0 };

    std::wstring m_userDataFolder;

    Microsoft::WRL::ComPtr<ICoreWebView2Environment> m_env;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller>  m_controller;
    Microsoft::WRL::ComPtr<ICoreWebView2>            m_webview;

    std::function<void()> m_onReady;
};

#endif // EDGEVIEW_H
