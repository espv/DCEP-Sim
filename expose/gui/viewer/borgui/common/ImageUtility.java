package borgui.common;

import javax.swing.*;
import java.awt.*;

public class ImageUtility {
    public static ImageIcon LoadOperatorImage(String filename) {
        ImageIcon icon = new ImageIcon(filename);
        Image newimg = icon.getImage().getScaledInstance(50, 50,  java.awt.Image.SCALE_SMOOTH);
        return new ImageIcon(newimg);
    }
}
