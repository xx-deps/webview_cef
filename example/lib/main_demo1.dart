import 'dart:async';
import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:webview_cef/webview_cef.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  late WebViewController _controller;

  final _textController = TextEditingController();
  String title = "";

  @override
  void initState() {
    super.initState();

    // 创建 webview
    _controller = WebviewManager().createWebView(
        loading: const Text("not initialized"));
        
    initPlatformState();
  }

  @override
  void dispose() {
    _controller.dispose();
    WebviewManager().quit();
    super.dispose();
  }

  // 初始化 Webview 并设置相关事件监听
  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    await WebviewManager().initialize(userAgent: "test/userAgent");
    String url = "www.pub.dev";
    _textController.text = url;
    //unified interface for all platforms set user agent
    _controller.setWebviewListener(WebviewEventsListener(
      onUrlChanged: (url) {
        _textController.text = url;
      },
      onLoadStart: (controller, url) {
        print("onLoadStart => $url");
      },
      onLoadEnd: (controller, url) {
        print("onLoadEnd => $url");
      },
    ));

    await _controller.initialize(_textController.text);

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData(useMaterial3: true),
      home: Scaffold(
          body: Column(
        children: [
          SizedBox(
            height: 20,
            child: Text(
              title,
              style: const TextStyle(color: Colors.blue),
            ),
          ),
          Row(
            children: [
              SizedBox(
                height: 48,
                child: MaterialButton(
                  onPressed: () {
                    _controller.reload();
                  },
                  child: const Icon(Icons.refresh),
                ),
              ),
              SizedBox(
                height: 48,
                child: MaterialButton(
                  onPressed: () {
                    _controller.goBack();
                  },
                  child: const Icon(Icons.arrow_left),
                ),
              ),
              SizedBox(
                height: 48,
                child: MaterialButton(
                  onPressed: () {
                    _controller.goForward();
                  },
                  child: const Icon(Icons.arrow_right),
                ),
              ),
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
                child: TextField(
                  controller: _textController,
                  onSubmitted: (url) {
                    _controller.loadUrl(url);
                  },
                ),
              ),
            ],
          ),
          Expanded(
              child: Row(
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
      )),
    );
  }
}
