#include "edgeview.h"

#include <cassert>
#include <utility>
#include <shlobj.h>
#include <knownfolders.h>

using Microsoft::WRL::ComPtr;

namespace
{
    inline bool EndsWithBackslash(const std::wstring& s)
    {
        return !s.empty() && (s.back() == L'\\' || s.back() == L'/');
    }
}

EdgeView::EdgeView() = default;

EdgeView::~EdgeView()
{
    if (m_controller)
    {
        m_controller->Close();
    }
    m_webview.Reset();
    m_controller.Reset();
    m_env.Reset();
}

HRESULT EdgeView::EnsureUserDataFolder()
{
    PWSTR path = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &path);
    if (FAILED(hr)) return hr;
    std::wstring base(path);
    CoTaskMemFree(path);
    if (!EndsWithBackslash(base)) base.append(L"\\");
    m_userDataFolder = base + L"edgeview";
    CreateDirectoryW(m_userDataFolder.c_str(), nullptr);

    return S_OK;
}

HRESULT EdgeView::Initialize(HWND parent)
{
    RECT rc{};
    if (!GetClientRect(parent, &rc))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return Initialize(parent, rc);
}

HRESULT EdgeView::Initialize(HWND parent, const RECT& bounds)
{
    m_parent = parent;
    m_bounds = bounds;

    HRESULT hr = EnsureUserDataFolder();
    if (FAILED(hr)) return hr;

    return CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        m_userDataFolder.c_str(),
        nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result) || !env) {
                    return S_OK;
                }
                m_env = env;
                return env->CreateCoreWebView2Controller(
                    m_parent,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (controller) {
                                m_controller = controller;
                                if (m_controller) {
                                    m_controller->get_CoreWebView2(&m_webview);
                                }
                                if (m_controller) {
                                    m_controller->put_IsVisible(TRUE);
                                    m_controller->put_Bounds(m_bounds);
                                }
                                OnReady();
                            }
                            return S_OK;
                        }
                        ).Get()
                    );
            }
            ).Get()
        );
}

void EdgeView::OnReady()
{
    if (m_onReady)
    {
        m_onReady();
    }
}

void EdgeView::SetReadyCallback(std::function<void()> cb)
{
    m_onReady = std::move(cb);
    if (IsReady() && m_onReady)
    {
        m_onReady();
    }
}

HRESULT EdgeView::Navigate(const wchar_t* url) const
{
    if (!m_webview) return E_POINTER;
    return m_webview->Navigate(url);
}

HRESULT EdgeView::SetBounds(const RECT& bounds)
{
    m_bounds = bounds;
    if (!m_controller) return E_POINTER;
    return m_controller->put_Bounds(m_bounds);
}

void EdgeView::ResizeToClient()
{
    if (!m_parent) return;
    RECT rc{};
    if (GetClientRect(m_parent, &rc))
    {
        SetBounds(rc);
    }
}

HRESULT EdgeView::SetVisible(BOOL visible)
{
    if (!m_controller) return E_POINTER;
    return m_controller->put_IsVisible(visible);
}
