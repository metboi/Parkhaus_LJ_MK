#include <random>
#include <string>

class SwissLicensePlateGenerator {
public:
  SwissLicensePlateGenerator() {
    std::random_device rd;
    generator = std::mt19937(rd());
    letterDistribution = std::uniform_int_distribution<int>('A', 'Z');
    digitDistribution = std::uniform_int_distribution<int>(0, 9);
  }

  std::string generateLicensePlate() {
    std::string licensePlate;

    // Generate random letters (2 uppercase letters)
    for (int i = 0; i < 2; ++i) {
      char letter = static_cast<char>(letterDistribution(generator));
      licensePlate += letter;
    }

    // Generate random numbers (3 digits)
    for (int i = 0; i < 3; ++i) {
      int digit = digitDistribution(generator);
      licensePlate += std::to_string(digit);
    }

    return licensePlate;
  }

private:
  std::mt19937 generator;
  std::uniform_int_distribution<int> letterDistribution;
  std::uniform_int_distribution<int> digitDistribution;
};
