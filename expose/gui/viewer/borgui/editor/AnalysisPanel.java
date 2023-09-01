package borgui.editor;

import borgui.client.Client;
import borgui.common.DebugTrace;
import borgui.common.SmartScroller;
import borgui.common.SwingPanel;
import borgui.graph.BorealisGraph;
import borgui.graph.BorealisGraphCell;
import borgui.graph.GraphOverviewPanel;
import borgui.visual.PlotColor;
import borgui.visual.ServerButtonPanel;
import borgui.xml.*;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import com.fasterxml.jackson.dataformat.yaml.YAMLMapper;
import com.github.underscore.U;
import org.jdom2.Document;
import org.jdom2.Element;
import org.jgraph.event.GraphSelectionEvent;
import org.jgraph.event.GraphSelectionListener;
import org.yaml.snakeyaml.Yaml;

import javax.swing.*;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.text.DefaultCaret;
import javax.swing.tree.TreePath;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.image.BufferedImage;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * A class that represent the editor panel to edit the digram.
 */
public class AnalysisPanel extends JPanel implements Editor
{
    protected Client              m_client;
    protected BorealisModel       m_borealisModel;

    protected JPanel              imagePanel = new JPanel();

     ///////////////////////////////////////////////////////////////////////////
     //
     // Creates a new instance of EditorPanel.
     //
     public AnalysisPanel(Client client)
     {
     //.........................................................................


        super();

        m_client = client;
        client.analysisPanel = this;

         Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
         int width = (int) screenSize.getWidth()-75;
         int height = (int) screenSize.getHeight()-75;
         GridLayout layout = new GridLayout(0, 10);
         layout.setHgap(0); // or whatever number of pixels you want
         layout.setVgap(0); // same
         this.imagePanel.setLayout(layout);
         JScrollPane scrollPane = new JScrollPane(imagePanel);
         scrollPane.setPreferredSize(new Dimension(width, height));
         scrollPane.setMinimumSize(new Dimension(width, height));
         //imagePanel.setPreferredSize(new Dimension(width, height));
         //imagePanel.setMinimumSize(new Dimension(width, height));
         scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
         scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
         this.add(scrollPane);

        this.setVisible(true);
    }


    public void showFigures() {
        File dir = new File("../../Results/figures");
        File[] directoryListing = dir.listFiles();
        if (directoryListing == null) {
            System.out.println("Results folder cannot be found!");
            System.exit(11);
        }
        for (File child : directoryListing) {
            String filename = child.getName();
            if (filename.endsWith(".png") || filename.endsWith(".jpg")) {
                int width = 200;
                int height = 200;
                BufferedImage bi;
                try {
                    ImageIcon ii = new ImageIcon(child.getAbsolutePath());//path to image
                    bi = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
                    Graphics2D g2d = bi.createGraphics();
                    g2d.addRenderingHints(new RenderingHints(RenderingHints.KEY_RENDERING,RenderingHints.VALUE_RENDER_QUALITY));
                    g2d.drawImage(ii.getImage(), 0, 0, width, height, null);
                    JPanel panel = new JPanel();
                    JLabel label = new JLabel();
                    Image dimg = bi.getScaledInstance(width, height, Image.SCALE_SMOOTH);
                    label.setIcon(new ImageIcon(dimg));
                    panel.add(label);
                    imagePanel.add(panel);
                    this.m_client.pack();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
        updateUI();
    }

    public BorealisModel getModel()
    {
        return m_borealisModel;
    }
}


/////////////////////////  end EditorPanel.java  ///////////////////////////////
