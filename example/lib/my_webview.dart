import 'package:flutter/material.dart';
import 'package:webview_cef/webview_cef.dart';
import 'package:webview_cef/src/webview_inject_user_script.dart';

class MyWebview extends StatefulWidget {
  final String url;
  const MyWebview({super.key, required this.url});

  @override
  State<MyWebview> createState() => _MyWebviewState();
}

class _MyWebviewState extends State<MyWebview> {
  late WebViewController _controller;

  @override
  void initState() {
    super.initState();
    // 注入js脚本
    var injectUserScripts = InjectUserScripts();

    // 页面开始的时候, 执行的脚本
    injectUserScripts
        .add(UserScript("console.log('页面加载开始')", ScriptInjectTime.LOAD_START));

    // 页面结束的时候, 执行的脚本
    injectUserScripts
        .add(UserScript("console.log('页面加载结束')", ScriptInjectTime.LOAD_END));

    // 创建 webview
    _controller = WebviewManager().createWebView(
        loading: const Column(children: [
          Text('loading'),
          CircularProgressIndicator(),
        ],),
        injectUserScripts: injectUserScripts);

    _initializeWebView();
  }

  @override
  void dispose() {
    _controller.dispose();
    WebviewManager().quit();
    super.dispose();
  }

  // 初始化 Webview 并设置相关事件监听
  Future<void> _initializeWebView() async {
    await WebviewManager().initialize(userAgent: "test/userAgent");

    _controller.setWebviewListener(WebviewEventsListener(
      onTitleChanged: (t) {
        setState(() {});
      },
      onUrlChanged: (url) {
        // JavascriptChannel 用于 JavaScript 和 Dart 之间的通信，监听从 JavaScript 发来的消息。
        final Set<JavascriptChannel> jsChannels = {
          JavascriptChannel(
              name: 'Print',
              onMessageReceived: (JavascriptMessage message) {
                print("JS say:" + message.message); // 打印 JavaScript 发来的消息
                _controller.sendJavaScriptChannelCallBack(
                    false,
                    "{'code':'200','message':'print succeed!'}",
                    message.callbackId,
                    message.frameId);
              }),
        };
        //normal JavaScriptChannels
        _controller.setJavaScriptChannels(jsChannels);
        _controller.executeJavaScript("window.Print('ee', 'rr')");
        //also you can build your own jssdk by execute JavaScript code to CEF
        _controller.executeJavaScript("function abc(e){return 'abc:'+ e}");
        _controller
            .evaluateJavascript("abc('zj_test')")
            .then((value) => print(value));
      },
      onLoadStart: (controller, url) {
        print("onLoadStart => $url");
      },
      onLoadEnd: (controller, url) {
        print("onLoadEnd => $url");
      },
    ));

    await _controller.initialize(widget.url);

    if (!mounted) return;
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Expanded(
            child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            ValueListenableBuilder(
              valueListenable: _controller,
              builder: (context, value, child) {
                return _controller.value
                    ? Expanded(child: _controller.webviewWidget)
                    : _controller.loadingWidget;
              },
            )
          ],
        ))
      ],
    );
  }
}
