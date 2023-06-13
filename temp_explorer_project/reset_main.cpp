#include <fstream>
#include <vector>
#include "reset.h"

int main() {
    std::ifstream input("C:\\Users\\jamil\\projects\\OpenBugbear\\game\\out\\data\\reset1.bin", std::ios::binary );
	
    unsigned objectCount = 0;
    input.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount));

    std::vector<ResetObject> objects(objectCount);

    for(auto& object: objects) {
        input.read(reinterpret_cast<char*>(&object), sizeof(object));
    }

    input.close();
    
    return 0;
}
