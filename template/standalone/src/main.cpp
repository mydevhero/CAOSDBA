#include <string>
#include <Repository.hpp>

#ifdef QUERY_EXISTS_IQuery_Demo_Test_echoString
int main(int argc, char* argv[])
{
  auto caos = std::make_unique<Caos>(argc, argv);

  auto ret = caos->repository->IQuery_Demo_Test_echoString("Hi CAOS :) ");

  if (ret.has_value())
  {
    std::cout << ret.value() << "\n";
  }
  else
  {
    std::cout << "CAOS\n";
  }

  return 0;
}
#else
int main(int argc, char* argv[])
{
  std::cout << "Configure IQuery_Demo_Test_echoString in query_definitions.txt" << "\n";

  return 0;
}
#endif
