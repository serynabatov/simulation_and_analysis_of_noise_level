package components.entities;

import lombok.Data;

@Data
public class Position {

    private float x;
    private float y;
    private float value;
    private boolean exceeded;
    private String timestamp;
}
