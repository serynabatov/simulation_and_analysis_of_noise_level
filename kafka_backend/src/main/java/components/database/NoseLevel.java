package components.database;

import java.sql.*;

public class NoseLevel extends Thread {

    private String noiseLevel = null;
    private float db;

    public NoseLevel(float db) {
        this.db = db;
    }

    private void getPoints(float db) {
        Connection conn = null;
        Statement stmt = null;
        String value = null;
        try {
            Class.forName(DatabaseConnectionUtilities.JDBC_DRIVER);

            conn = DriverManager.getConnection(DatabaseConnectionUtilities.DB_URL, DatabaseConnectionUtilities.USER, DatabaseConnectionUtilities.PASS);

            stmt = conn.createStatement();

            String sql = "SELECT level FROM NOISE_LEVEL WHERE LEFT_DB <= " + db + " AND RIGHT_DB >= " + db;

            ResultSet rs = stmt.executeQuery(sql);

            while (rs.next()) {
                value = rs.getString("level");
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

        if (value == null) {
            value = "Extremely loud";
        }

        noiseLevel = value;
    }

    public String getNoiseLevel() {
        return this.noiseLevel;
    }

    @Override
    public void run() {
        try {
            getPoints(this.db);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
