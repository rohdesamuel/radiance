# Radiance Game Engine
This game engine is a developed form of the Entity Component System. 

## Dependencies
```
sudo apt-get install libsdl2-2.0
sudo apt-get install libsdl2-dev
sudo apt-get install libglew-dev
```
## Design

### Source
An object that exposes a Reader class that emits elements upon execution.

### Sink
An object that exposes a Writer class that takes in elements upon execution.

### Frame
A shared piece of memory that holds temporary data between function executions.

### System
A single unit of execution that reads/writes to/from a Stack Frame.

### Pipeline
A class that encapsulates a queue of Systems to execute reading from a Source to a Sink.


