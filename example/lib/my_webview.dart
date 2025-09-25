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
