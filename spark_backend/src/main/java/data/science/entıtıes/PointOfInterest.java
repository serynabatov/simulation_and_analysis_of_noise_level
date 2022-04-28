package data.science.entıtıes;

import data.science.udt.PointOfInterestUDT;
import lombok.AllArgsConstructor;
import lombok.Data;
import org.apache.spark.sql.types.DataTypes;
import org.apache.spark.sql.types.SQLUserDefinedType;

import java.io.Serializable;

@Data
@AllArgsConstructor
@SQLUserDefinedType(udt = PointOfInterestUDT.class)
public class PointOfInterest implements Serializable {

    private int id;
    private float lat;
    private float lon;
    private String name;

    @Override
    public String toString() {
        return "{" +
                "\"id\": " + id + ", " +
                "\"lat\": " + lat + ", " +
                "\"lon\": " + lon + ", " +
                "\"name\": " + name + "" +
                "}";
    }

    public static PointOfInterest valueOf(int id, float lat, float lon, String name) {
        return new PointOfInterest(id, lat, lon, name);
    }


}
