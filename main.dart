import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'firebase_options.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:syncfusion_flutter_gauges/gauges.dart';
import 'manual.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'MDA SC',
      home: MyHomePage(title: 'MDA AUTO CAR'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});
  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  String result = "";
  String result2 = "";
  String color2 = "";
  String color3 = "";
  String state2 = "";
  String mode = "";
  double a = 0.0;
  final database = FirebaseDatabase.instance.ref();
  final _database = FirebaseDatabase.instance.ref();
  final _database2 = FirebaseDatabase.instance.ref();
  final _database3 = FirebaseDatabase.instance.ref();
  final database4 = FirebaseDatabase.instance.ref();
  FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin =
      FlutterLocalNotificationsPlugin();

  @override
  void initState() {
    super.initState();
    _initializeNotifications();
    _activatedListeners();
  }

  void _initializeNotifications() async {
    const AndroidInitializationSettings initializationSettingsAndroid =
        AndroidInitializationSettings('@mipmap/ic_launcher');
    final InitializationSettings initializationSettings =
        InitializationSettings(
      android: initializationSettingsAndroid,
    );
    await flutterLocalNotificationsPlugin.initialize(initializationSettings);
  }

  void _showNotification(String title, String body) async {
    const AndroidNotificationDetails androidPlatformChannelSpecifics =
        AndroidNotificationDetails(
      'your_channel_id',
      'your_channel_name',
      channelDescription: 'your_channel_description',
      importance: Importance.max,
      priority: Priority.high,
      showWhen: false,
    );
    const NotificationDetails platformChannelSpecifics =
        NotificationDetails(android: androidPlatformChannelSpecifics);
    await flutterLocalNotificationsPlugin.show(
      0,
      title,
      body,
      platformChannelSpecifics,
      payload: 'item x',
    );
  }

  void _activatedListeners() {
    _database.child('/baseSpeed').onValue.listen((DatabaseEvent event) {
      final speed = event.snapshot.value;
      setState(() {
        result = '$speed';
        a = double.parse(result);
        _showNotification('Tốc độ cập nhật', 'Tốc độ hiện tại là $result');
      });
    });
    _database2.child('/color').onValue.listen((DatabaseEvent event) {
      final color = event.snapshot.value;
      setState(() {
        color2 = '$color';
        if (color2 == 'yellow') {
          color3 = 'Vàng';
        }
        if (color2 == 'green') {
          color3 = 'Xanh Lá';
        }
        if (color2 == 'red') {
          color3 = 'Đỏ';
        }
        _showNotification(
            'Màu sắc cập nhật', 'Màu sắc vạch hiện tại là $color3');
      });
    });
    _database3.child('/state').onValue.listen((DatabaseEvent event) {
      final state = event.snapshot.value;
      setState(() {
        result2 = '$state';
        if (result2 == "stop") {
          state2 = "Đang dừng";
        }
        if (result2 == "forward") {
          state2 = "Đang di chuyển";
        }
        if (result2 == "obstacle") {
          state2 = "Gặp vật cản";
        }
        _showNotification(
            'Trạng thái cập nhật', 'Trạng thái hiện tại là $state2');
      });
    });
  }

  Color _getColorFromString(String color) {
    switch (color) {
      case 'red':
        return Colors.red;
      case 'yellow':
        return Colors.yellow;
      case 'green':
        return Colors.green;
      default:
        return Colors.white;
    }
  }

  @override
  Widget build(BuildContext context) {
    final usersRef = database.child("/");
    return Scaffold(
        appBar: AppBar(
          backgroundColor: Colors.transparent, // Make app bar transparent
          elevation: 0, // Remove shadow
          title: Text(
            widget.title,
            style: const TextStyle(
              color: Colors.deepPurple, // Title text color
              fontSize: 20, // Title text size
              fontWeight: FontWeight.bold, // Title text weight
            ),
          ),
          centerTitle: true, // Center the title
        ),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Container(
                width: 390,
                height: 100,
                decoration: BoxDecoration(
                  boxShadow: const [
                    BoxShadow(
                      color: Color.fromARGB(255, 175, 219, 255),
                      offset: Offset(
                        5.0,
                        5.0,
                      ),
                      blurRadius: 10.0,
                      spreadRadius: 4.0,
                    ), //BoxShadow
                    BoxShadow(
                      color: Colors.white,
                      offset: Offset(0.0, 0.0),
                      blurRadius: 0.0,
                      spreadRadius: 0.0,
                    ),
                  ],
                  borderRadius: BorderRadius.circular(20),
                ),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Text(
                      'Màu sắc',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 18,
                        color: Colors.deepPurple, // Adjusted font size
                      ),
                    ),
                    const Padding(padding: EdgeInsets.only(top: 5)),
                    Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                      Container(
                        width: 260,
                        height: 30,
                        decoration: BoxDecoration(
                          boxShadow: [
                            //BoxShadow
                            BoxShadow(
                              color: _getColorFromString(color2),
                              offset: const Offset(0.0, 0.0),
                              blurRadius: 0.0,
                              spreadRadius: 0.0,
                            ),
                          ],
                          borderRadius: BorderRadius.circular(20),
                        ),
                      ),
                    ]),
                  ],
                ),
              ),
              const Padding(padding: EdgeInsets.all(10)),
              Container(
                width: 390,
                height: 200,
                decoration: BoxDecoration(
                  boxShadow: const [
                    BoxShadow(
                      color: Color.fromARGB(255, 175, 219, 255),
                      offset: Offset(
                        5.0,
                        5.0,
                      ),
                      blurRadius: 10.0,
                      spreadRadius: 4.0,
                    ), //BoxShadow
                    BoxShadow(
                      color: Colors.white,
                      offset: Offset(0.0, 0.0),
                      blurRadius: 0.0,
                      spreadRadius: 0.0,
                    ),
                  ],
                  borderRadius: BorderRadius.circular(20),
                ),
                child: SfRadialGauge(
                  enableLoadingAnimation: false,
                  animationDuration: 2000,
                  axes: <RadialAxis>[
                    RadialAxis(
                      canScaleToFit: true,
                      showLabels: false,
                      showTicks: false,
                      minimum: 0,
                      maximum: 100,
                      radiusFactor:
                          0.9, // Adjusted to fit within a 200x200 container
                      axisLineStyle: const AxisLineStyle(
                        cornerStyle: CornerStyle.bothCurve,
                        thickness: 0.3,
                        thicknessUnit: GaugeSizeUnit.factor,
                      ),
                      pointers: <GaugePointer>[
                        RangePointer(
                          cornerStyle: CornerStyle.bothCurve,
                          enableAnimation: true,
                          value: a,
                          width: 0.2,
                          sizeUnit: GaugeSizeUnit.factor,
                          gradient: const SweepGradient(
                            colors: <Color>[
                              Color(0xFFCC2B5E),
                              Color(0xFF753A88)
                            ],
                            stops: <double>[0.25, 0.75],
                          ),
                        ),
                      ],
                      annotations: <GaugeAnnotation>[
                        GaugeAnnotation(
                          axisValue: 50,
                          positionFactor:
                              0.0, // Adjusted for centering within the gauge
                          widget: Column(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: [
                              const Text(
                                'Tốc độ',
                                style: TextStyle(
                                  fontWeight: FontWeight.bold,
                                  fontSize: 18,
                                  color:
                                      Colors.deepPurple, // Adjusted font size
                                ),
                              ),
                              const Padding(padding: EdgeInsets.only(top: 5)),
                              Row(
                                  mainAxisAlignment: MainAxisAlignment.center,
                                  children: [
                                    Text(
                                      '$a',
                                      style: const TextStyle(
                                        fontWeight: FontWeight.bold,
                                        fontSize: 23, // Adjusted font size
                                      ),
                                    ),
                                  ]),
                            ],
                          ),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
              const Padding(padding: EdgeInsets.all(10)),
              Container(
                width: 390,
                height: 100,
                decoration: BoxDecoration(
                  boxShadow: const [
                    BoxShadow(
                      color: Color.fromARGB(255, 175, 219, 255),
                      offset: Offset(
                        5.0,
                        5.0,
                      ),
                      blurRadius: 10.0,
                      spreadRadius: 4.0,
                    ), //BoxShadow
                    BoxShadow(
                      color: Colors.white,
                      offset: Offset(0.0, 0.0),
                      blurRadius: 0.0,
                      spreadRadius: 0.0,
                    ),
                  ],
                  borderRadius: BorderRadius.circular(20),
                ),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Text(
                      'Trạng thái',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 18,
                        color: Colors.deepPurple, // Adjusted font size
                      ),
                    ),
                    const Padding(padding: EdgeInsets.only(top: 5)),
                    Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                      Text(
                        state2,
                        style: const TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize: 23, // Adjusted font size
                        ),
                      ),
                    ]),
                  ],
                ),
              ),
              const Padding(padding: EdgeInsets.all(10)),
              Container(
                height: 60,
                width: 390,
                decoration: BoxDecoration(
                  boxShadow: const [
                    BoxShadow(
                      color: Color.fromARGB(255, 175, 219, 255),
                      offset: Offset(
                        5.0,
                        5.0,
                      ),
                      blurRadius: 10.0,
                      spreadRadius: 4.0,
                    ),
                    BoxShadow(
                      color: Colors.white,
                      offset: Offset(0.0, 0.0),
                      blurRadius: 0.0,
                      spreadRadius: 0.0,
                    ),
                  ],
                  borderRadius: BorderRadius.circular(20),
                ),
                child: Material(
                  color: Colors.transparent,
                  child: InkWell(
                    splashColor: Colors.cyan.withOpacity(0.2),
                    hoverColor: Colors.cyan.withOpacity(0.1),
                    highlightColor: Colors.cyan.withOpacity(0.2),
                    onTapDown: (_) {
                      usersRef.update({'mode': 'manual', 'command': 's'});
                      Navigator.push(
                        context,
                        MaterialPageRoute(builder: (context) => ManualScreen()),
                      );
                    },
                    borderRadius: BorderRadius.circular(20),
                    child: const Center(
                      child: Text(
                        'Chuyển sang Manual Mode',
                        style: TextStyle(
                            fontSize: 20,
                            fontWeight: FontWeight.bold,
                            color: Colors.deepPurple),
                      ),
                    ),
                  ),
                ),
              )
            ],
          ),
        ));
  }
}
