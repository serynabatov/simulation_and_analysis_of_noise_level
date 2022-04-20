package components;

import com.google.gson.Gson;
import components.database.NoseLevel;
import components.database.PointOfInterest;
import components.database.TimeOfDay;
import components.entities.EnrichedPointOfInterest;
import components.entities.Position;

import java.sql.Timestamp;
import java.time.Instant;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

public class DatabaseManager {

    private Gson g = new Gson();
    private Position position;

    public DatabaseManager(String value) {
        position = g.fromJson(value, Position.class);
    }

    public String start() throws InterruptedException {

        EnrichedPointOfInterest enrichedPointOfInterest = new EnrichedPointOfInterest();
        enrichedPointOfInterest.setX(position.getX());
        enrichedPointOfInterest.setY(position.getY());
        enrichedPointOfInterest.setDb(position.getValue());
        enrichedPointOfInterest.setExceeded(position.isExceeded());
        enrichedPointOfInterest.setTimestamp(position.getTimestamp());

        // parsing timestamp for time of the day
        Instant instant = Instant.ofEpochSecond(Long.parseLong(position.getTimestamp()));
        Date date = Date.from(instant);
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(date);

        NoseLevel noiseLevel = new NoseLevel(position.getValue());
        PointOfInterest pointOfInterest = new PointOfInterest(position.getX(), position.getY());
        TimeOfDay timeOfDay = new TimeOfDay(calendar.get(Calendar.HOUR_OF_DAY));

        noiseLevel.start();
        pointOfInterest.start();
        timeOfDay.start();

        noiseLevel.join();
        pointOfInterest.join();
        timeOfDay.join();

        List<components.entities.PointOfInterest> values = pointOfInterest.getValues();
        enrichedPointOfInterest.setPointOfInterest(values);

        String dbLevel = noiseLevel.getNoiseLevel();
        enrichedPointOfInterest.setNoiseLevel(dbLevel);

        String timeOfTheDay = timeOfDay.getValue();
        enrichedPointOfInterest.setTimeOfTheDay(timeOfTheDay);

        return enrichedPointOfInterest.toString();
    }

}
