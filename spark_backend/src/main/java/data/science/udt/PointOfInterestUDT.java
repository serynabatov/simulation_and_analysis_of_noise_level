package data.science.udt;

import data.science.entıtıes.PointOfInterest;
import org.apache.spark.sql.catalyst.InternalRow;
import org.apache.spark.sql.catalyst.expressions.GenericInternalRow;
import org.apache.spark.sql.types.*;
import org.apache.spark.unsafe.types.UTF8String;

public class PointOfInterestUDT extends UserDefinedType<PointOfInterest> {

    private static final DataType DATA_TYPE;
    private static final int ID_INDEX = 0;
    private static final int LAT_INDEX = 1;
    private static final int LON_INDEX = 2;
    private static final int NAME_INDEX = 3;

    static {
        MetadataBuilder metadataBuilder = new MetadataBuilder();
        metadataBuilder.putLong("maxNumber", 99);
        DATA_TYPE = DataTypes.createStructType(new StructField[]{
                DataTypes.createStructField("id", DataTypes.IntegerType, false, metadataBuilder.build()),
                DataTypes.createStructField("lat", DataTypes.FloatType, false),
                DataTypes.createStructField("lon", DataTypes.FloatType, false, metadataBuilder.build()),
                DataTypes.createStructField("name", DataTypes.StringType, false, metadataBuilder.build()),
        });
    }

    @Override
    public DataType sqlType() {
        return DATA_TYPE;
    }

    @Override
    public Object serialize(PointOfInterest pointOfInterest) {
        InternalRow row = new GenericInternalRow(4);
        row.setInt(ID_INDEX, pointOfInterest.getId());
        row.update(LAT_INDEX, pointOfInterest.getLat());
        row.update(LON_INDEX, pointOfInterest.getLon());
        row.update(NAME_INDEX, UTF8String.fromString(pointOfInterest.getName()));
        return row;
    }

    @Override
    public PointOfInterest deserialize(Object datum) {
        if (datum instanceof InternalRow) {
            InternalRow row = (InternalRow) datum;
            return PointOfInterest.valueOf(row.getInt(ID_INDEX), row.getFloat(LAT_INDEX), row.getFloat(LON_INDEX), row.getString(NAME_INDEX));
        }
        throw new IllegalStateException("Unsupported conversion");
    }

    @Override
    public Class<PointOfInterest> userClass() {
        return PointOfInterest.class;
    }
}
