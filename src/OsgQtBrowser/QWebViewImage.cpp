/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include "OsgQtBrowser/QWebViewImage.h"

QWebViewImage::QWebViewImage()
{
	// make sure we have a valid QApplication before we start creating widgets.
	getOrCreateQApplication();

	_webView = new QWebView;

	_webPage = new QWebPage;
	_webPage->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
	_webPage->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

	_webView->setPage(_webPage);

	// Add loadFinished listener to ensure our page is loaded before assigning data objects
	connect(_webPage->mainFrame(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

	_adapter = new QGraphicsViewAdapter(this, _webView.data());
}

void QWebViewImage::loadFinished(bool ok)
{
	// Hack: in previous attempts we were unable to pass array to javascript
	// Therefore we loop through selected models arrays and add array item by item

	// Clear qData object with empty models array attribute
	_webPage->mainFrame()->evaluateJavaScript("var qData={models:[]};");

	// Iterate over all models and add them one by one to models array
	QList<Lua::LuaGraphTreeModel * >::iterator i;
	Lua::LuaGraphTreeModel *model;

	// Iterate over each selected node
	for (i = _models->begin(); i != _models->end(); i++){
		model = *i;

		// Pass current model to javascript temp variable
		_webPage->mainFrame()->addToJavaScriptWindowObject("tempModelItem", model);

		// Send signal to javascript that item could be added to qData.models
		// Note: sadly, this needs to be implemented in javascript
		_webPage->mainFrame()->evaluateJavaScript("tempModelItemReady();");
	}

	// Send signal to browser that qData is ready
	_webPage->mainFrame()->evaluateJavaScript("qDataReady();");
}

void QWebViewImage::navigateTo(const std::string& url)
{
	_webView->load(QUrl(url.c_str()));
}

void QWebViewImage::focusBrowser(bool focus)
{
	QFocusEvent event(focus ? QEvent::FocusIn : QEvent::FocusOut, Qt::OtherFocusReason);
	QCoreApplication::sendEvent(_webPage, &event);
}

void QWebViewImage::clearWriteBuffer()
{
	_adapter->clearWriteBuffer();
}

void QWebViewImage::render()
{
	_adapter->render();
}

void QWebViewImage::setFrameLastRendered(const osg::FrameStamp* frameStamp)
{
	_adapter->setFrameLastRendered(frameStamp);
}

bool QWebViewImage::sendPointerEvent(int x, int y, int buttonMask)
{
	return _adapter->sendPointerEvent(x,y,buttonMask);
}

bool QWebViewImage::sendKeyEvent(int key, bool keyDown)
{
	return QWebViewImage::_adapter->sendKeyEvent(key, keyDown);
}

