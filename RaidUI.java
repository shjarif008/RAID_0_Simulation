import javax.swing.*;
import java.awt.*;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.File;

public class RaidUI extends JFrame {
    private JTextArea log;
    private JTextField writeField;

    public RaidUI() {
        setTitle("RAID 0 Simulator");
        setSize(700, 450);
        setLocationRelativeTo(null);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        log = new JTextArea();
        log.setEditable(false);
        log.setFont(new Font("Monospaced", Font.PLAIN, 12));
        JScrollPane scroll = new JScrollPane(log);
        scroll.setBorder(BorderFactory.createTitledBorder("RAID Log"));

        writeField = new JTextField(30);
        writeField.setToolTipText("Data to write (spaces allowed)");

        JButton init = new JButton("Init");
        JButton write = new JButton("Write");
        JButton read = new JButton("Read");
        JButton inspect = new JButton("Inspect");

        init.addActionListener(e -> run("init"));
        read.addActionListener(e -> run("read"));
        inspect.addActionListener(e -> run("inspect"));
        write.addActionListener(e -> {
            String text = writeField.getText();
            if (text.isEmpty()) {
                log("Nothing to write");
                return;
            }
            run("write " + text);
            writeField.setText("");
        });

        JPanel bottom = new JPanel(new FlowLayout(FlowLayout.LEFT));
        bottom.add(new JLabel("Write:"));
        bottom.add(writeField);
        bottom.add(write);
        bottom.add(init);
        bottom.add(read);
        bottom.add(inspect);

        setLayout(new BorderLayout());
        add(scroll, BorderLayout.CENTER);
        add(bottom, BorderLayout.SOUTH);

        log("RAID UI started");

        checkInit();
    }

    private void checkInit() {
        try {
            File exe = new File("raid0.exe").getAbsoluteFile(); 
            File workingDir = new File(".").getAbsoluteFile();

            ProcessBuilder pb = new ProcessBuilder(exe.getAbsolutePath(), "status");
            pb.directory(workingDir);
            pb.redirectErrorStream(true);

            Process p = pb.start();
            BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));

            boolean needsInit = false;
            String line;
            while ((line = br.readLine()) != null) {
                log(line);
                if (line.toLowerCase().contains("action required: init")) {
                    needsInit = true;
                }
            }
            p.waitFor();

            if (needsInit) {
                log("RAID not initialized. Please click 'Init' to initialize the array.");
            }

            log(""); 

        } catch (Exception e) {
            log("ERROR: " + e.getMessage());
        }
    }

    private void run(String cmd) {
        log("Executing: " + cmd);
        try {
            File exe = new File("raid0.exe").getAbsoluteFile(); 
            File workingDir = new File(".").getAbsoluteFile();

            ProcessBuilder pb = new ProcessBuilder(exe.getAbsolutePath(), cmd);
            pb.directory(workingDir);
            pb.redirectErrorStream(true);

            Process p = pb.start();
            BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));

            String line;
            while ((line = br.readLine()) != null) {
                log(line);
            }

            p.waitFor();

            log("");

        } catch (Exception e) {
            log("ERROR: " + e.getMessage());
        }
    }

    private void log(String m) {
        log.append(m + "\n");
        log.setCaretPosition(log.getDocument().getLength());
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new RaidUI().setVisible(true));
    }
}

