package borgui.client;

import borgui.editor.AnalysisPanel;
import borgui.editor.EditorPanel;

import javax.swing.*;
import java.awt.*;

/**
 * The main borealis gui frame. Individual applications can be added in
 * as a tabbed component.
 *
 */
public class Client extends JFrame {
    
    protected JTabbedPane m_mainTabbedPane;
    protected JMenu m_fileMenu;
    protected JMenu m_viewMenu;

    public EditorPanel editorPanel;
    public AnalysisPanel analysisPanel;

    /**
     *Creates a new instance of Client.
     *
     *@param title the title of the frame.
     */
    public Client(String title) {
        super(title);
        // main tabbed pane
        m_mainTabbedPane = new JTabbedPane();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        m_mainTabbedPane.setPreferredSize(screenSize);
        m_mainTabbedPane.addChangeListener(e -> {
            Dimension dimension = getSize();
            m_mainTabbedPane.setPreferredSize(dimension);
            pack();
        });

        // menu bar
        JMenuBar menuBar = new JMenuBar();
        m_viewMenu = new JMenu("View");
        m_fileMenu = new JMenu("File");
        menuBar.add(m_fileMenu);
        menuBar.add(m_viewMenu);
        this.setJMenuBar(menuBar);

        //delete menu
       /*
        m_editMenu = new JMenu("Edit");
        menuBar.add(m_editMenu);
        */
        // frame layout
        this.getContentPane().setLayout(new BorderLayout());
        this.getContentPane().add(m_mainTabbedPane, BorderLayout.CENTER);
        
        // display editor window
        JFrame.setDefaultLookAndFeelDecorated(true);
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.pack();
        this.setVisible(true);
    }
    
    
    /**
     * Adds a tab containing the passed in Component to the main borealis
     * gui frame.
     * @param  title the title text to be displayed in this tab
     * @param  icon the icon to be displayed in this tab
     * @param  component the component to be displayed when this tab is clicked
     * @param  tip the tooltip to be displayed for this tab
     * @see    JTabbedPane
     */
    public void addTab(String title, Icon icon, Component component, String tip) {
        m_mainTabbedPane.addTab(title, icon, component, tip);
    }
    
// THIS MENU ITEM SYSTEM MAY BE CHANGED!!!!
    
    
    /**
     * Adds a JMenuItem to the main window's "file" menu.
     *
     * @param  menuItem item to be added to the main window's "file" menu
     * @see    JMenu
     */
    public void addFileMenuItem(JMenuItem menuItem) {
        m_fileMenu.add(menuItem);
    }
    
    /**
     * Adds a JMenuItem to the main window's "edit" menu.
     *
     * @param  menuItem item to be added to the main window's "edit" menu
     * @see    JMenu
     */
    
    //delete menu
    /*
    public void addEditMenuItem(JMenuItem menuItem) {
        m_editMenu.add(menuItem);
    }
     */
    /**
     * Adds a JMenuItem to the main windows "view" menu.
     *
     * @param  menuItem item to be added to the main window's "view" menu
     * @see    JMenu
     */
    public void addViewMenuItem(JMenuItem menuItem) {
        m_viewMenu.add(menuItem);
    }
    
}



