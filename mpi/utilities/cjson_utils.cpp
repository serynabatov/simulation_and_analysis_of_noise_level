#include <cjson_utils.h>


void create_json(cJSON* message, double x, double y, double value, double exceeded, char* timestamp)
{
    cJSON_AddNumberToObject(message, "x", x);
    cJSON_AddNumberToObject(message, "y", y);
    cJSON_AddNumberToObject(message, "value", value);
    cJSON_AddBoolToObject(message, "exceeded", exceeded);
    cJSON_AddStringToObject(message, "timestamp", timestamp);
}