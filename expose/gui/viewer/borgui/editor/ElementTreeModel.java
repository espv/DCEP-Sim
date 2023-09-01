package borgui.editor;

import javax.swing.text.AttributeSet;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.*;
import java.util.List;
import java.util.Vector;
import java.util.Iterator;
import org.jdom2.*;
import borgui.xml.*;

/**
 * A class that defines a suitable data model for a Element Tree.
 */
public class ElementTreeModel  implements TreeModel
{
    private Vector<TreeModelListener>  m_treeModelListeners =
                                            new Vector<TreeModelListener>();

    public Element m_root;
    
    /**
     * Constructor.
     */
    public ElementTreeModel(BorealisElement element)
    {
        m_root = element;
    }
    
    
    /**
     * Set the Element for this model.
     */
    public void setElement(Element element)
    {
        m_root = element;
        showTreeUpdate();
    }
    
    /**
     * Set the Element for this model.
     */
    public void setElement(BorealisElement element)
    {
        m_root = element;
        showTreeUpdate();
    }
    
    
    /**
     * Update the tree.
     */
    public void showTreeUpdate()
    {
        fireTreeStructureChanged(m_root);
    }
    
//////////////// Fire events //////////////////////////////////////////////
    
    /**
     * The only event raised by this model is TreeStructureChanged with the
     * root as path, i.e. the whole tree has changed.
     */
    protected void fireTreeStructureChanged(Object oldRoot)
    {
        int len = m_treeModelListeners.size();

        TreeModelEvent e = new TreeModelEvent(this,
                                              new Object[] {oldRoot});

        for (int i = 0; i < len; i++)
        {
            ((TreeModelListener)m_treeModelListeners.elementAt(i)).treeStructureChanged(e);
        }
    }


    protected void fireTreeNodesInserted(Object source, Object[] path,
                                         int[] childIndices,
                                         Object[] children) {
        // Guaranteed to return a non-null array
        TreeModelEvent e = null;
        int len = m_treeModelListeners.size();
        // Process the listeners last to first, notifying
        // those that are interested in this event
        for (int i = 0; i < len; i++) {
            // Lazily create the event:
            if (e == null)
                e = new TreeModelEvent(source, path,
                        childIndices, children);
            m_treeModelListeners.elementAt(i).treeNodesInserted(e);
        }
    }


    public Element[] getPathToRoot(Element aNode) {
        return getPathToRoot(aNode, 0);
    }

    protected Element[] getPathToRoot(Element aNode, int depth) {
        Element[]              retNodes;
        // This method recurses, traversing towards the root in order
        // size the array. On the way back, it fills in the nodes,
        // starting from the root and working back to the original node.

        /* Check for null, in case someone passed in a null node, or
           they passed in an element that isn't rooted at root. */
        if(aNode == null) {
            if(depth == 0)
                return null;
            else
                retNodes = new Element[depth];
        }
        else {
            depth++;
            if(aNode == m_root)
                retNodes = new Element[depth];
            else
                retNodes = getPathToRoot(aNode.getParentElement(), depth);
            retNodes[retNodes.length - depth] = aNode;
        }
        return retNodes;
    }


    public void nodesWereInserted(Element node, int[] childIndices) {
        if(m_treeModelListeners != null && node != null && childIndices != null
                && childIndices.length > 0) {
            int               cCount = childIndices.length;
            Object[]          newChildren = new Object[cCount];

            /*for (Attribute attribute : node.getAttributes()) {
                newChildren[counter] = node.getContent().get(node.getContentSize()-1);
            }*/
            //int counter = 0;
            //for(; counter < node.getAttributesSize(); counter++)
            //    newChildren[counter] = node.getAttributes().get(childIndices[counter]);
            //for(; counter < cCount; counter++)
            //    newChildren[counter] = node.getChildren().get(node.getChildren().size()-1);
            Element[] pathToRoot = getPathToRoot(node);
            fireTreeNodesInserted(this, pathToRoot, new int[]{node.getChildren().size()-1},
                    new Element[]{node.getChildren().get(node.getChildren().size()-1)});
        }
    }


    public void insertNodeInto(Element newChild,
                               Element parent, int index){
        // Before, addContent was called without an index, which doesn't make sense to me
        parent.addContent(index, newChild);

        int[]           newIndexs = new int[1];

        newIndexs[0] = index;
        nodesWereInserted(parent, newIndexs);
    }
    
    
//////////////// TreeModel interface implementation ///////////////////////
    
    /**
     * Adds a listener for the TreeModelEvent posted after the tree changes.
     */
    public void addTreeModelListener(TreeModelListener l)
    {
        m_treeModelListeners.addElement(l);
    }
    

    /**
     * Returns the child of parent at index index in the parent's child array.
     */
    public Object getChild(Object parent, int index)
    {
        
        if (isLeaf(parent))
        {
            return null;
        }
        
        List<Object>  children  = new Vector<Object>();
        Element element = (Element)parent;

        //children.addAll(element.getAttributes());
        //children.addAll(element.getChildren());

        for(Iterator i = element.getAttributes().iterator(); i.hasNext();)
        {
            children.add(i.next());
        }


        for(Iterator i = element.getChildren().iterator(); i.hasNext();)
        {
            children.add(i.next());
        }
        
        if (index < 0 || index >= children.size())
        {
            return null;
        }
        
        return children.get(index);
    }
    

    /**
     * Returns the number of children of parent.
     */
    public int getChildCount(Object parent)
    {
        if (isLeaf(parent))
        {
            return 0;
        }
        
        Element element = (Element)parent;
        return element.getAttributes().size() + element.getChildren().size();
    }

    
    /**
     * Returns the index of child in parent.
     */
    public int getIndexOfChild(Object parent, Object child)
    {
        Element element = (Element) parent;
        List<Object> children = new Vector<Object>();

        //children.addAll(element.getAttributes());
        //children.addAll(element.getChildren());

        for(Iterator i = element.getAttributes().iterator(); i.hasNext();)
        {
            children.add(i.next());
        }


        for(Iterator i = element.getChildren().iterator(); i.hasNext();)
        {
            children.add(i.next());
        }
 
        
        return children.indexOf(child);
    }

    
    /**
     * Returns the root of the tree.
     */
    public Object getRoot()
    {
        return m_root;
    }

    
    /**
     * Returns true if node is a leaf.
     */
    public boolean isLeaf(Object node)
    {
        return node instanceof Attribute;
    }

    
    /**
     * Removes a listener previously added with addTreeModelListener().
     */
    public void removeTreeModelListener(TreeModelListener l)
    {
        m_treeModelListeners.removeElement(l);
    }

    
    /**
     * Messaged when the user has altered the value for the item
     * identified by path to newValue.  Not used by this model.
     */
    public void valueForPathChanged(TreePath path, Object newValue)
    {
        System.out.println("*** valueForPathChanged : "
                + path + " --> " + newValue);
        fireTreeStructureChanged(m_root);
    }
}


/////////////////////////  end ElementTreeModel.java  //////////////////////////
