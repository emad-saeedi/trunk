# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./plugins/DynamicEngine/SDECDynamicEngine
# Target is a library:  

LIBS += -lSerialization \
        -lSDECDiscreteElement \
        -lRigidBody \
        -lSphere \
        -lSDECContactModel \
        -lMath \
        -lBody \
        -lEngine \
        -lGeometry \
        -lInteraction \
        -lMultiMethods \
        -lSDECPermanentLink \
        -lSDECPermanentLinkPhysics \
        -lSDECContactPhysics \
        -rdynamic 
INCLUDEPATH = $(YADEINCLUDEPATH) 
MOC_DIR = $(YADECOMPILATIONPATH) 
UI_DIR = $(YADECOMPILATIONPATH) 
OBJECTS_DIR = $(YADECOMPILATIONPATH) 
QMAKE_LIBDIR = ../../../toolboxes/Libraries/Serialization/$(YADEDYNLIBPATH) \
               ../../../plugins/Body/SDECDiscreteElement/$(YADEDYNLIBPATH) \
               ../../../plugins/Body/RigidBody/$(YADEDYNLIBPATH) \
               ../../../plugins/GeometricalModel/Sphere/$(YADEDYNLIBPATH) \
               ../../../plugins/InteractionGeometry/SDECContactModel/$(YADEDYNLIBPATH) \
               ../../../toolboxes/Libraries/Math/$(YADEDYNLIBPATH) \
               ../../../yade/Body/$(YADEDYNLIBPATH) \
               ../../../yade/Engine/$(YADEDYNLIBPATH) \
               ../../../yade/Geometry/$(YADEDYNLIBPATH) \
               ../../../yade/Interaction/$(YADEDYNLIBPATH) \
               ../../../toolboxes/Libraries/MultiMethods/$(YADEDYNLIBPATH) \
               ../../../plugins/InteractionGeometry/SDECPermanentLink/$(YADEDYNLIBPATH) \
               ../../../plugins/InteractionPhysics/SDECPermanentLinkPhysics/$(YADEDYNLIBPATH) \
               ../../../plugins/InteractionPhysics/SDECContactPhysics/$(YADEDYNLIBPATH) \
               ../../../plugins/InteractionModel/SDECContactModel/$(YADEDYNLIBPATH) \
               ../../../yade/MultiMethods/$(YADEDYNLIBPATH) \
               ../../../plugins/InteractionModel/SDECPermanentLink/$(YADEDYNLIBPATH) \
               $(YADEDYNLIBPATH) 
QMAKE_CXXFLAGS_RELEASE += -lpthread \
                          -pthread 
QMAKE_CXXFLAGS_DEBUG += -lpthread \
                        -pthread 
DESTDIR = $(YADEDYNLIBPATH) 
CONFIG += debug \
          warn_on \
          dll 
TEMPLATE = lib 
HEADERS += SDECDynamicEngine.hpp 
SOURCES += SDECDynamicEngine.cpp 
