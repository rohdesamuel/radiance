#ifndef PROGRAM__H
#define PROGRAM__H

#include "pipeline.h"

namespace radiance
{

// A list of pipelines to execute in-order synchronously.
typedef std::vector<BasePipeline*> Process;

// An editable list of pipelines to execute.
class Program {
 public:
  // Run the program.
  Status operator()(void); 

  // Create/remove a new process to run asynchronously from each other.
  Status create_process();
  Status remove_process();
};

}  // namespace radiance

#endif  // #ifndef PROGRAM__H
