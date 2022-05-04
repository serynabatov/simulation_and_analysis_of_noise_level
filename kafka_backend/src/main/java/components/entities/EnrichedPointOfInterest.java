package components.entities;

import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

@Data
@NoArgsConstructor
public class EnrichedPointOfInterest {

    private float x;
    private float y;
    private float db;
    private boolean exceeded;
    private String timestamp;
    private List<PointOfInterest> pointOfInterest;
    private String noiseLevel;
    private String timeOfTheDay;

    @Override
    public String toString() {

        StringBuilder points = new StringBuilder();
        points.append("[ ");

        for (PointOfInterest pointOfInterest1 : pointOfInterest) {
            points.append(pointOfInterest1.toString());
        }
        points.append(" ]");

        return "{" +
                "\"latitude\": " + x + ", " +
                "\"longitude\": " + y + ", " +
                "\"db\": " + db + ", " +
                "\"exceeded\": " +  exceeded + ", " +
                "\"timestamp\": " + timestamp + ", " +
                "\"pointOfInterest\": " + points.toString() + ", " +
                "\"noiseLevel\": " + "\"" + noiseLevel + "\"" + ", " +
                "\"timeOfTheDay\": " + "\"" + timeOfTheDay +  "\"" +
                "}";
    }

}
