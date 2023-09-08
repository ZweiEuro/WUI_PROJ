#pragma once

#include "include/cef_client.h"

#include "RenderHandler.hpp"

namespace WUI
{

    // Client actually handling the html internals
    class BrowserClient : public CefClient
    {
    public:
        CefRefPtr<CefRenderHandler> m_renderHandler;

    public:
        BrowserClient(CefRefPtr<WUI::RenderHandler> &renderHandler)
            : m_renderHandler(renderHandler)
        {
        }

        virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
        {
            return m_renderHandler;
        }

        IMPLEMENT_REFCOUNTING(BrowserClient);
    };
}