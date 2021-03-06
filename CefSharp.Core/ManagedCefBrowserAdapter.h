// Copyright © 2010-2015 The CefSharp Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "Stdafx.h"

#include <include/cef_runnable.h>

#include "BrowserSettings.h"
#include "PaintElementType.h"
#include "Internals/ClientAdapter.h"
#include "Internals/CefDragDataWrapper.h"
#include "Internals/RenderClientAdapter.h"
#include "Internals/MCefRefPtr.h"
#include "Internals/StringVisitor.h"
#include "Internals/CefFrameWrapper.h"
#include "Internals/CefSharpBrowserWrapper.h"

using namespace CefSharp::Internals;
using namespace System::Diagnostics;
using namespace System::ServiceModel;
using namespace System::Threading;
using namespace System::Threading::Tasks;

namespace CefSharp
{
    public ref class ManagedCefBrowserAdapter : public DisposableResource, IBrowserAdapter
    {
        MCefRefPtr<ClientAdapter> _clientAdapter;
        BrowserProcessServiceHost^ _browserProcessServiceHost;
        IWebBrowserInternal^ _webBrowserInternal;
        JavascriptObjectRepository^ _javaScriptObjectRepository;
        IBrowser^ _browserWrapper;

    private:
        // Private keyboard functions:
        bool IsKeyDown(WPARAM wparam)
        {
            return (GetKeyState(wparam) & 0x8000) != 0;
        }

        // Misc. private functions:
        int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);
        CefMouseEvent GetCefMouseEvent(MouseEvent^ mouseEvent);

    protected:
        virtual void DoDispose(bool isDisposing) override
        {
            CloseAllPopups(true);
            Close(true);

            _clientAdapter = nullptr;

            // Guard managed only member derefs by isDisposing:
            if (isDisposing && _browserProcessServiceHost != nullptr)
            {
                _browserProcessServiceHost->Close();
                _browserProcessServiceHost = nullptr;
            }

            _webBrowserInternal = nullptr;
            _javaScriptObjectRepository = nullptr;

            delete _browserWrapper;
            _browserWrapper = nullptr;

            DisposableResource::DoDispose(isDisposing);
        };

    public:
        ManagedCefBrowserAdapter(IWebBrowserInternal^ webBrowserInternal, bool offScreenRendering)
        {
            if (offScreenRendering)
            {
                _clientAdapter = new RenderClientAdapter(webBrowserInternal, this);
            }
            else
            {
                _clientAdapter = new ClientAdapter(webBrowserInternal, this);
            }

            _webBrowserInternal = webBrowserInternal;
            _javaScriptObjectRepository = gcnew JavascriptObjectRepository();
        }

        virtual void OnAfterBrowserCreated(int browserId);
        void CreateOffscreenBrowser(IntPtr windowHandle, BrowserSettings^ browserSettings, String^ address);
        void CreateBrowser(BrowserSettings^ browserSettings, IntPtr sourceHandle, String^ address);
        void Close(bool forceClose);
        void CloseAllPopups(bool forceClose);
        void LoadUrl(String^ address);
        void WasResized();
        void WasHidden(bool hidden);
        void Invalidate(PaintElementType type);
        void SendFocusEvent(bool isFocused);
        void SetFocus(bool isFocused);
        bool SendKeyEvent(int message, int wParam, int lParam);
        void OnMouseMove(int x, int y, bool mouseLeave, CefEventFlags modifiers);
        void OnMouseButton(int x, int y, int mouseButtonType, bool mouseUp, int clickCount, CefEventFlags modifiers);
        void OnMouseWheel(int x, int y, int deltaX, int deltaY);
        virtual Task<JavascriptResponse^>^ EvaluateScriptAsync(int browserId, Int64 frameId, String^ script, Nullable<TimeSpan> timeout);
        virtual Task<JavascriptResponse^>^ EvaluateScriptAsync(String^ script, Nullable<TimeSpan> timeout);
        void Resize(int width, int height);
        void NotifyMoveOrResizeStarted();
        void NotifyScreenInfoChanged();
        void RegisterJsObject(String^ name, Object^ object, bool lowerCaseJavascriptNames);
        void OnDragTargetDragEnter(CefDragDataWrapper^ dragData, MouseEvent^ mouseEvent, DragOperationsMask allowedOperations);
        void OnDragTargetDragOver(MouseEvent^ mouseEvent, DragOperationsMask allowedOperations);
        void OnDragTargetDragLeave();
        void OnDragTargetDragDrop(MouseEvent^ mouseEvent);

        ///
        // Returns the main (top-level) frame for the browser window.
        ///
        IFrame^ GetMainFrame();

        ///
        // Returns the focused frame for the browser window.
        ///
        /*--cef()--*/
        IFrame^ GetFocusedFrame();

        ///
        // Returns the frame with the specified identifier, or NULL if not found.
        ///
        /*--cef(capi_name=get_frame_byident)--*/
        IFrame^ GetFrame(System::Int64 identifier);

        ///
        // Returns the frame with the specified name, or NULL if not found.
        ///
        /*--cef(optional_param=name)--*/
        IFrame^ GetFrame(String^ name);

        ///
        // Returns the number of frames that currently exist.
        ///
        /*--cef()--*/
        int GetFrameCount();

        ///
        // Returns the identifiers of all existing frames.
        ///
        /*--cef(count_func=identifiers:GetFrameCount)--*/
        List<System::Int64>^ GetFrameIdentifiers();

        ///
        // Returns the names of all existing frames.
        ///
        /*--cef()--*/
        List<String^>^ GetFrameNames();

        /// <summary>
        /// Gets the CefBrowserWrapper instance
        /// </summary>
        /// <returns>Gets the current instance or null</returns>
        IBrowser^ GetBrowser();
    };
}
