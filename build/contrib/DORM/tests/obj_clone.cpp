#include "Test.hpp"

int main() {
	Test t;
	t.name("Fudge");
	t.age(5);

	auto cloned_t = t.clone();

	std::cout << "Clone's name: " << cloned_t->name() << std::endl;

	if ( cloned_t->name() != t.name() )
		throw std::runtime_error("Clone failed");

	exit(0);
}
