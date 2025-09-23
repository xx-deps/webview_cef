import 'package:flutter/material.dart';
import 'package:webview_cef_example/my_webview.dart';

class WebViewTabs extends StatefulWidget {
  const WebViewTabs({super.key});

  @override
  State<WebViewTabs> createState() => _WebViewTabsState();
}

class _WebViewTabsState extends State<WebViewTabs>
    with TickerProviderStateMixin {
  late TabController _tabController;

  @override
  void initState() {
    super.initState();
    _tabController = TabController(length: 3, vsync: this);
    _tabController.addListener(() {
      setState(() {}); // tab 改变时重建
    });
  }

  @override
  void dispose() {
    _tabController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    Widget child;

    switch (_tabController.index) {
      case 0:
        child = APage();
        break;
      case 1:
        child =
            MyWebview(url: "https://pub.dev/packages/webview_cef/changelog");
        break;
      case 2:
        child = MyWebview(url: "http://127.0.0.1");
        break;
      default:
        child = const SizedBox.shrink();
    }
    return Scaffold(
      appBar: AppBar(
        title: const Text('WebView Tabs'),
        bottom: TabBar(
          controller: _tabController,
          tabs: const [
            Tab(text: 'Tab 1'),
            Tab(text: 'Tab 2'),
            Tab(text: 'Tab 3'),
          ],
        ),
      ),
      body: child,
    );
  }
}

class APage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    print("APage build");
    return const Center(child: Text("Page A"));
  }
}

class CPage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    print("CPage build");
    return const Center(child: Text("Page C"));
  }
}
