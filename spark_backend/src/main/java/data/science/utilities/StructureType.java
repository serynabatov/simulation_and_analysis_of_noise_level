package data.science.utilities;

import data.science.udt.PointOfInterestUDT;
import org.apache.spark.sql.types.DataTypes;
import org.apache.spark.sql.types.StructType;

public class StructureType {

    public static StructType pointSchemaCreate() {
        return new StructType()
                .add("latitude", DataTypes.FloatType, true)
                .add("longitude", DataTypes.FloatType, true)
                .add("db", DataTypes.FloatType, true)
                .add("exceeded", DataTypes.BooleanType, true)
                .add("timestamp", DataTypes.LongType, true)
                .add("pointOfInterest", DataTypes.createArrayType(new PointOfInterestUDT()), true)
                .add("noiseLevel", DataTypes.StringType, true)
                .add("timeOfTheDay", DataTypes.StringType,  false);
    }

    public static StructType pointOfInterestSchemaCreate() {
        return new StructType()
                .add("id", DataTypes.IntegerType, true)
                .add("lat", DataTypes.FloatType, true)
                .add("lon", DataTypes.FloatType, true)
                .add("name", DataTypes.StringType, true);
    }

}
