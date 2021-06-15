import os
import git
from git import Repo
from PyQt5.QtWidgets import * 
from PyQt5 import QtCore, QtGui
from PyQt5.QtGui import * 
from PyQt5.QtCore import * 
import sys

print("config git submodules")
repo = Repo(os.getcwd())
print(repo)
subms = repo.submodules



class gitcontrolgui(QMainWindow):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setWindowTitle("arm-cortem-cmake-tempelate")
        self.UiComponents()
        
    def UiComponents(self):
  
        # creating a QListWidget
        list_widget = QListWidget(self)
  
        # setting geometry to it
        list_widget.setGeometry(50, 70, 150, 60)
  
        # list widget items
        item = []
        for submodule in subms:
            #print(submodule.update())
            if submodule.module_exists():
                list_widget.addItem(submodule.name)
        

     
  


if __name__=="__main__":
    app = QApplication(sys.argv)
    window = gitcontrolgui()
    window.show()
    sys.exit(app.exec_())