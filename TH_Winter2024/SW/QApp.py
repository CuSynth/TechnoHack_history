import sys
from random import randint

from PyQt6 import QtWebEngineWidgets, QtCore
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QLabel, QFrame, QPushButton, QSizePolicy
from PyQt6.QtWidgets import QVBoxLayout, QHBoxLayout, QGridLayout, QSpacerItem, QDial
from PyQt6.QtGui import QPalette, QColor, QPixmap
import pyqtgraph as pg

import folium




# pip show folium >> 0.12.1
# pip install folium --upgrade
# pip uninstall folium
# pip install folium==0.12.1



class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("My App")

        self.map_widget = None

        self.UpdateMapWidget()
        self.SetTimers()


        # Set all layouts
        widget = QWidget()
        widget.setLayout(self.CreateLayouts())
        self.setCentralWidget(widget)

    def SetTimers(self):
        # Add a timer to update map
        self.map_timer = QtCore.QTimer()
        self.map_timer.setInterval(1000)
        self.map_timer.timeout.connect(self.update_position)
        self.map_timer.start()

    def CreateLayouts(self):
        # Vertical:
        self.vert_layout_1 = QVBoxLayout()
        self.vert_layout_1.addWidget(self.map_widget, stretch=1)

        # Horizontal:
        self.hor_layout = QHBoxLayout()
        self.hor_layout.addLayout(self.vert_layout_1)
    
        # Border:
        self.hor_layout.setContentsMargins(3,3,3,3)
        # Between widgets:
        self.hor_layout.setSpacing(5)
        
        return self.hor_layout


    # # ------------------------------------------------------------------------
    # # Map:
# 55.02, 82.93
    def UpdateMapWidget(self, location = [0, 0], marker=False):
        # Create map and add marker
        self.location = location
    
        map = folium.Map(location=self.location, zoom_start=15)
        if marker:
            marker = folium.Marker(location=self.location).add_to(map)
    
        # Create map widget        
        if self.map_widget is None:
            self.map_widget = QtWebEngineWidgets.QWebEngineView()
        self.map_widget.setHtml(map.get_root().render())
        
        # self.map_widget.resize(320, 240)


    def update_position(self):
        # update location and redraw
        self.UpdateMapWidget(location=[self.location[0], self.location[1]], marker=True)        




if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = MainWindow()
    # window.resize(1200, 900)
    window.show()
    
    app.exec()

