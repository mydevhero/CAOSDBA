#include <string>
#include <Repository.hpp>

int main(int argc, char* argv[])
{
  auto caos = std::make_unique<Caos>(argc, argv);
  /*
  auto ret = caos->repository->IQuery_your_query("This is my first CAOS query");

  if (ret.has_value())
  {
    std::cout << ret.value() << "\n";
  }
  else
  {
    std::cout << "CAOS\n";
  }
  */
  return 0;
}
