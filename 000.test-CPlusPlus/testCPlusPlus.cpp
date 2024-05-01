#include <iostream>
#include <vector>




int main()
{
	std::vector<int> test_v;

	int t[10],j[10];
	for (int i = 0; i < 10; i++)
	{
		t[i] = i;
		j[i] = i;
		std::cout << t[i] << std::endl;
	}

	std::cout << "Hello world" << std::endl;

	test_v.assign(&t[1], &t[10]);

	for (int i = 0; i < test_v.size(); i++)
	{
		
		std::cout << test_v[i] << std::endl;
	}

	return 0;


}