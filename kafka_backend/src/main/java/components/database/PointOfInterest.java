package components.database;

import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public class PointOfInterest extends Thread {

    private float diffLan = 0.01f;
    private float diffLot = 0.01f;
    private float lat;
    private float lon;

    public PointOfInterest(float lat, float lon) {
        this.lat = lat;
        this.lon = lon;
    }

    private List<components.entities.PointOfInterest> values = new ArrayList<>();

    public void getPoints() {
        Connection conn = null;
        Statement stmt = null;

        try {
            Class.forName(DatabaseConnectionUtilities.JDBC_DRIVER);

            conn = DriverManager.getConnection(DatabaseConnectionUtilities.DB_URL, DatabaseConnectionUtilities.USER, DatabaseConnectionUtilities.PASS);

            stmt = conn.createStatement();

            String sql = "SELECT NAME from POINT_OF_INTEREST WHERE ABS(LAN - " + lat + ") < " + diffLan + " AND ABS(LAN - " + lon + ") < " + diffLot;

            ResultSet rs = stmt.executeQuery(sql);

            while (rs.next()) {

                int id = rs.getInt("id");
                float localLot = rs.getFloat("lot");
                float localLat = rs.getFloat("lan");
                String name = rs.getString("name");
                values.add(new components.entities.PointOfInterest(id, localLat, localLot, name));
            }
        } catch (Exception se) {
            se.printStackTrace();
        } finally {

            try {
                if (stmt != null) stmt.close();
            } catch (SQLException se) {
                se.printStackTrace();
            }

            try {
                if (conn != null) conn.close();
            } catch (SQLException se) {
                se.printStackTrace();
            }
        }

        if (values.size() == 0) {
            values.add(new components.entities.PointOfInterest(0, 0, 0, "UNKNOWN"));
        }

    }

    public List<components.entities.PointOfInterest> getValues() {
        return this.values;
    }

    @Override
    public void run() {
        try {
            getPoints();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
