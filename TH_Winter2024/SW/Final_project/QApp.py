import sys
from main import *

from random import randint
import crc
from PyQt6.QtWebEngineWidgets import *
import PyQt6
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QLabel
from PyQt6.QtWidgets import QVBoxLayout, QHBoxLayout, QFrame
from PyQt6.QtGui import QPalette, QColor
from PyQt6.QtCore import Qt, QUrl, QTimer
import pyqtgraph as pg

import folium

def isclose(a, b, rel_tol=1e-02, abs_tol=0.0):
    return abs(a-b) <= max(rel_tol * max(abs(a), abs(b)), abs_tol)

class Lable(QWidget):
    def __init__(self, parent=None):
        super(Lable, self).__init__(parent=parent)
        self.layoutUI()

    def set_text(self, text):
        self.text.setText(text)



    def set_value(self, value):
        self.value.setText(value)

    def layoutUI(self):

        self.frame = QFrame(self)

        self.frame.setFrameStyle(QFrame.Shape.Panel | QFrame.Shadow.Raised)
        self.frame.setLineWidth(10)

        self.verticalLayout = QVBoxLayout(self.frame)
        self.text = QLabel("Text")
        self.text.setMargin(20)
        self.text.setStyleSheet("QLabel {font: 20pt Comic Sans MS}")
        self.value = QLabel("Val")
        self.value.setMargin(20)
        self.value.setStyleSheet("QLabel {font: 33pt Comic Sans MS}")
        self.verticalLayout.addWidget(self.text)
        self.verticalLayout.addWidget(self.value)


        self.principalLayout = QHBoxLayout(self)
        self.principalLayout.addWidget(self.frame)

        self.setLayout(self.principalLayout)


class MainWindow(QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setWindowTitle("My App")

        self.map_widget = None
        self.map_width = None
        self.cntr = 0
        # self.location = [0, 0]
        self.UpdateMapWidget(launch=True, marker=True)
        self.CreateDynPlotWidget1()
        self.CreateDynPlotWidget2()
        self.CreateDynPlotWidget3()
        self.Clock()

        self.SetTimers()

        # Set all layouts
        widget = QWidget()
        widget.setLayout(self.CreateLayouts())
        self.setStyleSheet("QMainWindow {background: 'darkseagreen';}")
        self.setCentralWidget(widget)

        self.client = SocketClient("10.6.1.127", 7777)
        # client = SocketClient("localhost", 7777)

        self.client.received.subscribe(self.my_handler)
        self.client.connected.subscribe(lambda: print('Connected'))
        self.client.disconnected.subscribe(lambda: print('Discnnected'))
        self.client.connect()

    def my_handler(self, data):
        # ....
        presure, temperature, humidity,longitude, latitude, hours, minuts, seconds = my_handler(data)
        if presure==0 and temperature==0 and humidity==0 and longitude==0 and latitude==0 and hours==0 and minuts==0 and seconds==0:
            print("No Packs")
            return
        self.update_plot(presure, temperature, humidity,longitude, latitude, hours, minuts, seconds)
        # update_plot
        pass


    def request(self):
        try:

        #while True:
            in_data = bytearray([22]) #str = input('>')
            self.client.send(create_massage(in_data))

        except KeyboardInterrupt:
            self.client.disconnect()
            print('shutdown')

    def CreateLayouts(self):
        # Horizontal:
        self.hor_layout = QHBoxLayout()
        self.hor_layout_3_1 = QHBoxLayout()

        # Vertical:
        self.vert_layout_1 = QVBoxLayout()
        self.vert_layout_2 = QVBoxLayout()
        self.vert_layout_3 = QVBoxLayout()

        self.hor_layout.addLayout(self.vert_layout_1, stretch=3)
        self.hor_layout.addLayout(self.vert_layout_2, stretch=1)
        self.hor_layout.addLayout(self.vert_layout_3, stretch=3)

        self.vert_layout_1.addWidget(self.dyn_plot_graph_1, stretch=1)
        self.vert_layout_1.addWidget(self.dyn_plot_graph_2, stretch=1)
        self.vert_layout_1.addWidget(self.dyn_plot_graph_3, stretch=1)

        self.vert_layout_2.addWidget(self.layout_temperature, stretch=1)
        self.vert_layout_2.addWidget(self.layout_pressure, stretch=1)
        self.vert_layout_2.addWidget(self.layout_humidity, stretch=1)

        self.vert_layout_3.addWidget(self.map_widget, stretch=3)
        self.vert_layout_3.addLayout(self.hor_layout_3_1, stretch=1)

        self.hor_layout_3_1.addWidget(self.clock_all, stretch=1)
        self.hor_layout_3_1.addWidget(self.map_width, stretch=1)

        # Border:
        self.hor_layout.setContentsMargins(3,3,3,3)
        # Between widgets:
        self.hor_layout.setSpacing(5)

        return self.hor_layout

    # ------------------------------------------------------------------------
    # Plot:

    #def CreatePlotWidget(self):
        # Temperature vs time plot
        self.plot_graph = pg.PlotWidget()
        time = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        temperature = [30, 32, 34, 32, 33, 31, 29, 32, 35, 30]
        self.plot_graph.plot(time, temperature)

    def Clock(self):
        self.clock_all = Lable()
        self.clock_all.set_text("time")
        self.clock_all.setStyleSheet("background-color: darkseagreen")
        self.clock_all.set_value("")

    def CreateDynPlotWidget1(self):
        self.dyn_plot_graph_1 = pg.PlotWidget()
        self.dyn_plot_graph_1.setBackground("w")

        self.layout_temperature = Lable(self)
        self.layout_temperature.set_text("Temperature")
        self.layout_temperature.setStyleSheet("background-color: darkseagreen")

        # Titel
        self.dyn_plot_graph_1.setTitle("Temperature vs Time", color="b", size="20pt")

        # Axis title
        styles = {"color": "red", "font-size": "18px"}
        self.dyn_plot_graph_1.setLabel("left", "Temperature (°C)", **styles)
        self.dyn_plot_graph_1.setLabel("bottom", "Time (sec)", **styles)

        self.dyn_plot_graph_1.addLegend()
        self.dyn_plot_graph_1.showGrid(x=True, y=True)


        # self.dyn_plot_graph_1.setYRange(20, 40)

        # Set data variables
        self.time = list(range(10))
        self.temperature = [randint(20, 40) for _ in range(10)]

        # Get a line reference
        pen = pg.mkPen(color=(255, 0, 0))
        self.line = self.dyn_plot_graph_1.plot(
            self.time,
            self.temperature,
            name="Temperature Sensor",
            pen=pen,
            symbol="+",
            symbolSize=15,
            symbolBrush="b",
        )

    def CreateDynPlotWidget2(self):
        self.dyn_plot_graph_2 = pg.PlotWidget()
        #self.dyn_plot_graph_2.setBackground("w")

        self.layout_pressure = Lable(self)
        self.layout_pressure.set_text("Pressure")
        self.layout_pressure.setStyleSheet("background-color: darkseagreen")

        # Titel
        self.dyn_plot_graph_2.setTitle("Pressure vs Time", color="w", size="20pt")

        # Axis title
        styles = {"color": "red", "font-size": "18px"}
        self.dyn_plot_graph_2.setLabel("left", "Pressure (Pa)", **styles)
        self.dyn_plot_graph_2.setLabel("bottom", "Time (sec)", **styles)

        self.dyn_plot_graph_2.addLegend()
        self.dyn_plot_graph_2.showGrid(x=True, y=True)


        # self.dyn_plot_graph_2.setYRange(100, 300)

        # Set data variables
        self.time = list(range(10))
        self.pressure = [randint(100, 300) for _ in range(10)]


        # Get a line reference
        pen = pg.mkPen(color=(255, 0, 0))
        self.line_2 = self.dyn_plot_graph_2.plot(
            self.time,
            self.pressure,
            name="Pressure Sensor",
            pen=pen,
            symbol="+",
            symbolSize=15,
            symbolBrush="w",
        )

    def CreateDynPlotWidget3(self):
        self.dyn_plot_graph_3 = pg.PlotWidget()
        self.dyn_plot_graph_3.setBackground("w")

        self.layout_humidity = Lable(self)
        self.layout_humidity.set_text("Humidity")
        self.layout_humidity.setStyleSheet("background-color: darkseagreen")

        # self.variable_humidity = QLabel("")
        # self.variable_humidity.setMargin(20)
        # self.variable_humidity.setStyleSheet("QLabel {font: 33pt Comic Sans MS}")

        # self.text_humidity = QLabel("Humidity")
        # self.text_humidity.setMargin(20)
        # self.text_humidity.setStyleSheet("QLabel {font: 15pt Comic Sans MS}")

        # Titel
        self.dyn_plot_graph_3.setTitle("Humidity vs Time", color="b", size="20pt")

        # Axis title
        styles = {"color": "red", "font-size": "18px"}
        self.dyn_plot_graph_3.setLabel("left", "Humidity (%)", **styles)
        self.dyn_plot_graph_3.setLabel("bottom", "Time (sec)", **styles)

        self.dyn_plot_graph_3.addLegend()
        self.dyn_plot_graph_3.showGrid(x=True, y=True)


        # self.dyn_plot_graph_3.setYRange(0, 100)

        # Set data variables
        self.time = list(range(10))
        self.Humidity = [randint(0, 100) for _ in range(10)]


        # Get a line reference
        pen = pg.mkPen(color=(255, 0, 0))
        self.line_3 = self.dyn_plot_graph_3.plot(
            self.time,
            self.Humidity,
            name="Humidity Sensor",
            pen=pen,
            symbol="+",
            symbolSize=15,
            symbolBrush="b",
        )

    def update_plot(self, presure, temperature, humidity,longitude, latitude, hours, minuts, seconds):
        self.cntr = self.cntr+1
        if (self.cntr % 4):
            return
        
        # update plot
        self.time = self.time[1:]
        self.time.append(self.time[-1] + 1)
        self.temperature = self.temperature[1:]
        temp = temperature[0]
        self.temperature.append(temp)
        self.layout_temperature.set_value("%.2f"%temp +"°C")

        temp2 = presure[0]
        self.pressure = self.pressure[1:]
        self.pressure.append(temp2)
        self.layout_pressure.set_value("%.2f"%temp2 +"mm")

        self.Humidity = self.Humidity[1:]
        temp3 = humidity[0]
        self.Humidity.append(temp3)
        self.layout_humidity.set_value("%.2f"%temp3 +"%")
        self.line.setData(self.time, self.temperature)
        self.line_2.setData(self.time, self.pressure)
        self.line_3.setData(self.time, self.Humidity)

        # self.clock = self.clock[1:]
        second = seconds
        min = minuts
        chas = hours
        self.clock_all.set_value("%02d"%(chas,) +":" + "%02d"%(min,)+":" + "%02d"%(second,))

        # update location and redraw
        wid = 54.843100111111111111
        lon = 83.09320011111111111
        # print("\n\n\n\n\n")
        # print(wid, lon)
        # print("\n\n\n\n\n")
        self.UpdateMapWidget(location=[wid, lon], marker=True)
        self.map_width.setText("%.6f"%wid +"°\n" + "%.6f"%lon + "°")

    # ------------------------------------------------------------------------
    # Map:

    def UpdateMapWidget(self, location = [54.843100111111111111, 83.09320011111111111], marker=False, launch=False):
        if not launch:
            print("Old: ", self.location, " New: ", location)

            if (isclose(self.location[0], location[0]) and isclose(self.location[1], location[1])) or (isclose(0, location[0]) and isclose(0, location[1])):
                print("ret")
                return
        
        print("Loc: _", location[0], "_", location[1], "_")
        print("Type: _", type(location[0]), "_", type(location[1]), "_")
        # Create map and add marker
        self.location = location
        if self.map_width is None:
            self.map_width = QLabel("")
            self.map_width.setMargin(20)
            self.map_width.setStyleSheet("QLabel {font: 28pt Comic Sans MS}")

        map = folium.Map(location=self.location, zoom_start=15, attr=" ")

        if marker:
            marker = folium.Marker(location=self.location).add_to(map)
        
        # # Create map widget
        if self.map_widget is None:
            self.map_widget = QWebEngineView()
        
        html = map.get_root().render()
        self.map_widget.setHtml(html)


    def SetTimers(self):
        # Add a timer to update plot
        self.plot_timer = QTimer()
        self.plot_timer.setInterval(200)
        self.plot_timer.timeout.connect(self.request)
        self.plot_timer.start()


# Test widget. Just to fill free space.
class Color(QWidget):
    def __init__(self, color, parent=None):
        super(Color, self).__init__(parent)
        self.setAutoFillBackground(True)
        palette = self.palette()
        palette.setColor(QPalette.ColorRole.Window, QColor(color))
        self.setPalette(palette)

if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()
    window.resize(1200, 900)
    app.exec()
