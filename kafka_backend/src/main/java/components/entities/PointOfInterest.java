package components.entities;

import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class PointOfInterest {

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

}
