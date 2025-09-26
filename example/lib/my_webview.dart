import 'package:flutter/material.dart';
import 'package:webview_cef/webview_cef.dart';

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

    // 创建 webview
    _controller = WebviewManager().createWebView(
      loading: const Column(
        children: [
          Text('loading'),
          CircularProgressIndicator(),
        ],
      ),
    );

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

    _controller.setWebviewListener(WebviewEventsListener());

    await _controller.initialize(widget.url);

    if (!mounted) return;
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        SizedBox(
          height: 48,
          child: MaterialButton(
            onPressed: () {
              _controller.openDevTools();
            },
            child: const Icon(Icons.developer_mode),
          ),
        ),
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
