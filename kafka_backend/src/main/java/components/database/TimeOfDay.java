package components.database;

import java.sql.*;

public class TimeOfDay extends Thread {

    private int hour;
    private String value = null;

    public TimeOfDay(int hour) {
        this.hour = hour;
    }

    public void getPoints() {
        Connection conn = null;
        Statement stmt = null;
        try {
            Class.forName(DatabaseConnectionUtilities.JDBC_DRIVER);

            conn = DriverManager.getConnection(DatabaseConnectionUtilities.DB_URL, DatabaseConnectionUtilities.USER, DatabaseConnectionUtilities.PASS);

            stmt = conn.createStatement();

            String sql = "SELECT name FROM TIME_OF_DAY WHERE LEFT_HOUR <= " + hour + " AND RIGHT_HOUR >= " + hour;

            ResultSet rs = stmt.executeQuery(sql);

            while (rs.next()) {
                value = rs.getString("name");
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
    }

    public String getValue() {
        return this.value;
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
