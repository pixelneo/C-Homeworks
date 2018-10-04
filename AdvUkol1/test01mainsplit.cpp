#include "test01split.hpp"

#include <iostream>
#include <istream>
#include <string>
#include <fstream>
#include <sstream>


int main(int argc, char * * argv)
{
	using namespace splitter;

    std::istringstream iss("C;;A;alCpha:=10/50.1");

	std::string x;
	int y;
	double z;
    char x1 = 'X',x2='Y',x3='Z';

    //iss >> split(x1,';',x2,';',x3,';',x, ':', '=', y, '/', z);
    
	//std::cout << "x = " << x << ", y = " << y << ", z = " << z << std::endl;

	if ( argc > 1 )
	{
       // std::ifstream ifs(argv[1]);
        std::ifstream ifs("rozvrh.csv");

		int count = 0;
		int sumdelka = 0;
		int sumlogtyden = 0;
		std::string sumtyp = "";

		for (;;)
		{
			std::string rl; 
			int delka; 
			std::string mistnost; 
			int den; 
			int cas; 
			int fakulta; 
			int skr; 
			int sem; 
			std::string tl; 
			int logtyden; 
			int pocettydnu; 
			char genotyp; 
			std::string gl; 
			std::string kod; 
			std::string oldkod; 
			char typ; 
			std::string paralelka; 
			std::string katedra; 
			std::string kod2;

            try {
                ifs >> split(rl, ';',
                    delka, ';',
                    mistnost, ';',
                    den, ';',
                    cas, ';',
                    fakulta, ';',
                    skr, ';',
                    sem, ';',
                    tl, ';',
                    logtyden, ';',
                    pocettydnu, ';',
                    genotyp, ';',
                    gl, ';',
                    kod, ';',
                    oldkod, ';',
                    typ, ';',
                    paralelka, ';',
                    katedra, ';',
                    kod2);

				++count;
                //std::cout <<"--- " << count << std::endl;
				sumdelka += delka * pocettydnu;
				sumlogtyden += logtyden;
				sumtyp.push_back( typ);
                std::cout << kod2 << std::endl;
			}
			catch (...)
			{
				if (ifs.eof())
					break;
				throw;
			}
		}
        std::ofstream of;
        of.open("out.txt");
        of << "count = " << count << ", sumdelka = " << sumdelka << ", sumlogtyden = " << sumlogtyden << std::endl;
        of << sumtyp << std::endl;
        of.close();
		std::cout << "count = " << count << ", sumdelka = " << sumdelka << ", sumlogtyden = " << sumlogtyden << std::endl;
		std::cout << sumtyp << std::endl;
	}

	return 0;
}

