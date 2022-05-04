#include <stringutility.h>

std::string tokenize(std::string s, std::string del) {

    int start = 0;
    int end = s.find(del);
    
    std::string values[2];
    
    int i = 0;
    
    while (end != -1)
    {
        values[i] = s.substr(start, end - start);
        start = end + del.size();
        end = s.find(del, start);
        i++;
    }
    
    values[i] = s.substr(start, end - start);

    return values[0].append(values[1].substr(0, 2));

}