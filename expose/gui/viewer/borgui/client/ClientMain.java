package borgui.client;

import javax.swing.*;
import java.awt.*;
import java.util.*;

import borgui.editor.*;
import borgui.visual.*;
import borgui.generator.*;
import borgui.xml.*;

/**
 * A class that include main method.
 */
public class ClientMain
{
    /**|
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event-dispatching thread.
     */
    private static Client createAndShowGUI(String[] args)
    {
        Client client;
        client = new Client("Borealis Client");
        
        EditorPanel editorPanel = new EditorPanel(client);
        //EditorPanel editorPanel = new EditorPanel(client);
        AnalysisPanel analysisPanel = new AnalysisPanel(client);
        VisualPanel visualPanel = new VisualPanel(editorPanel);
        InputGenPanel inputGenPanel = new InputGenPanel(client);
        //client.addTab("Visual", null, visualPanel, "visualize borealis query");
        client.addTab("Editor", null, editorPanel, "create/edit borealis query");
        client.addTab("Analysis", null, analysisPanel, "look at simulation results");
        //client.addTab("Input Generator", null, inputGenPanel, "generate borealis input");
        //client.addTab("Espen's editor", null, espenEditorPanel, "generate borealis input");

        
        // hack
        //if (args.length > 1)
        //{   visualPanel.load(args[0]);
        //    System.out.println("Loading Deployment File = " + args[1]);
        //}
        return client;
    }

    
    /**
     * main methond
     */
    public static void main(final String[] args)
    {
        // Schedule a job for the event-dispatching thread:
        // creating and showing this application's GUI.
        //
        javax.swing.SwingUtilities.invokeLater(() ->
        {
            Client client = createAndShowGUI(args);
            javax.swing.SwingUtilities.invokeLater(() ->
            {
                // Done setting everything up and can resize things
                EditorPanel editorComponent = (EditorPanel) client.m_mainTabbedPane.getComponent(0);
                AnalysisPanel analysisComponent = (AnalysisPanel) client.m_mainTabbedPane.getComponent(1);
                ButtonGroup buttonGroup = editorComponent.m_graphButtonPanel.m_buttonGroup;
                Enumeration<AbstractButton> buttons = buttonGroup.getElements();
                Dimension graphButtonPanelSize = editorComponent.m_graphButtonPanel.getSize();
                ButtonGroup newButtonGroup = new ButtonGroup();
                int number_buttons = buttonGroup.getButtonCount();
                int rows = (int) Math.ceil(number_buttons/GraphButtonPanel.COLS);
                editorComponent.m_graphButtonPanel.removeAll();
                for (int i = 0; i < number_buttons; i++) {
                    ImageIcon icon = new ImageIcon(editorComponent.m_graphButtonPanel.icons[i]);
                    int originalWidth = icon.getIconWidth();
                    int originalHeight = icon.getIconHeight();
                    double aspectRatio = originalWidth/originalHeight;
                    int newWidth = graphButtonPanelSize.width/GraphButtonPanel.COLS;
                    int newHeight = graphButtonPanelSize.height/rows;
                    double newAspectRatio = (double)newWidth/(double)newHeight;
                    if (newAspectRatio > aspectRatio) {
                        newWidth = newHeight * originalWidth/originalHeight;
                    } else if (newAspectRatio < aspectRatio) {
                        newHeight = newWidth * (originalHeight/originalWidth);
                    }
                    JRadioButton but = new JRadioButton(new ImageIcon(icon.getImage().getScaledInstance(newWidth, newHeight, 1)));
                    but.setActionCommand(Integer.toString(i));
                    but.addActionListener(editorComponent.m_graphButtonPanel.m_selectAction);
                    but.setBackground(EditorConstants.BACKGROUND_COLOR);
                    newButtonGroup.add(but);
                    editorComponent.m_graphButtonPanel.add(but);
                }
            });
        });
    }
}


/////////////////////////////  end  ClientMain.java  ///////////////////////////
