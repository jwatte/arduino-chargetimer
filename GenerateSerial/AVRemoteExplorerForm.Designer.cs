namespace GenerateSerial
{
    partial class AVRemoteExplorerForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.serialPortAVR = new System.IO.Ports.SerialPort(this.components);
            this.listBoxInfo = new System.Windows.Forms.ListBox();
            this.labelToRead = new System.Windows.Forms.Label();
            this.labelToWrite = new System.Windows.Forms.Label();
            this.timerPoll = new System.Windows.Forms.Timer(this.components);
            this.buttonReset = new System.Windows.Forms.Button();
            this.buttonStatus = new System.Windows.Forms.Button();
            this.textBoxCarrier = new System.Windows.Forms.TextBox();
            this.textBoxDutyCycle = new System.Windows.Forms.TextBox();
            this.buttonCarrier = new System.Windows.Forms.Button();
            this.textBoxTargetNEC = new System.Windows.Forms.TextBox();
            this.textBoxCmdNEC = new System.Windows.Forms.TextBox();
            this.buttonSendNEC = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // serialPortAVR
            // 
            this.serialPortAVR.BaudRate = 57600;
            this.serialPortAVR.DtrEnable = true;
            this.serialPortAVR.Handshake = System.IO.Ports.Handshake.RequestToSend;
            this.serialPortAVR.PortName = "COM3";
            this.serialPortAVR.ReadTimeout = 100;
            this.serialPortAVR.RtsEnable = true;
            this.serialPortAVR.WriteTimeout = 100;
            // 
            // listBoxInfo
            // 
            this.listBoxInfo.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listBoxInfo.ItemHeight = 16;
            this.listBoxInfo.Location = new System.Drawing.Point(119, 13);
            this.listBoxInfo.Name = "listBoxInfo";
            this.listBoxInfo.Size = new System.Drawing.Size(748, 532);
            this.listBoxInfo.TabIndex = 1;
            // 
            // labelToRead
            // 
            this.labelToRead.AutoSize = true;
            this.labelToRead.Location = new System.Drawing.Point(12, 13);
            this.labelToRead.Name = "labelToRead";
            this.labelToRead.Size = new System.Drawing.Size(46, 17);
            this.labelToRead.TabIndex = 2;
            this.labelToRead.Text = "label1";
            // 
            // labelToWrite
            // 
            this.labelToWrite.AutoSize = true;
            this.labelToWrite.Location = new System.Drawing.Point(12, 39);
            this.labelToWrite.Name = "labelToWrite";
            this.labelToWrite.Size = new System.Drawing.Size(46, 17);
            this.labelToWrite.TabIndex = 3;
            this.labelToWrite.Text = "label2";
            // 
            // timerPoll
            // 
            this.timerPoll.Enabled = true;
            this.timerPoll.Interval = 50;
            this.timerPoll.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // buttonReset
            // 
            this.buttonReset.Location = new System.Drawing.Point(15, 72);
            this.buttonReset.Name = "buttonReset";
            this.buttonReset.Size = new System.Drawing.Size(75, 23);
            this.buttonReset.TabIndex = 4;
            this.buttonReset.Text = "Reset";
            this.buttonReset.UseVisualStyleBackColor = true;
            this.buttonReset.Click += new System.EventHandler(this.buttonReset_Click);
            // 
            // buttonStatus
            // 
            this.buttonStatus.Location = new System.Drawing.Point(15, 110);
            this.buttonStatus.Name = "buttonStatus";
            this.buttonStatus.Size = new System.Drawing.Size(75, 23);
            this.buttonStatus.TabIndex = 5;
            this.buttonStatus.Text = "Status";
            this.buttonStatus.UseVisualStyleBackColor = true;
            this.buttonStatus.Click += new System.EventHandler(this.buttonStatus_Click);
            // 
            // textBoxCarrier
            // 
            this.textBoxCarrier.Location = new System.Drawing.Point(15, 150);
            this.textBoxCarrier.Name = "textBoxCarrier";
            this.textBoxCarrier.Size = new System.Drawing.Size(98, 22);
            this.textBoxCarrier.TabIndex = 6;
            this.textBoxCarrier.Text = "38000";
            // 
            // textBoxDutyCycle
            // 
            this.textBoxDutyCycle.Location = new System.Drawing.Point(13, 179);
            this.textBoxDutyCycle.Name = "textBoxDutyCycle";
            this.textBoxDutyCycle.Size = new System.Drawing.Size(100, 22);
            this.textBoxDutyCycle.TabIndex = 7;
            this.textBoxDutyCycle.Text = "33";
            // 
            // buttonCarrier
            // 
            this.buttonCarrier.Location = new System.Drawing.Point(15, 208);
            this.buttonCarrier.Name = "buttonCarrier";
            this.buttonCarrier.Size = new System.Drawing.Size(75, 23);
            this.buttonCarrier.TabIndex = 8;
            this.buttonCarrier.Text = "Carrier";
            this.buttonCarrier.UseVisualStyleBackColor = true;
            this.buttonCarrier.Click += new System.EventHandler(this.buttonCarrier_Click);
            // 
            // textBoxTargetNEC
            // 
            this.textBoxTargetNEC.Location = new System.Drawing.Point(15, 263);
            this.textBoxTargetNEC.Name = "textBoxTargetNEC";
            this.textBoxTargetNEC.Size = new System.Drawing.Size(98, 22);
            this.textBoxTargetNEC.TabIndex = 9;
            // 
            // textBoxCmdNEC
            // 
            this.textBoxCmdNEC.Location = new System.Drawing.Point(15, 292);
            this.textBoxCmdNEC.Name = "textBoxCmdNEC";
            this.textBoxCmdNEC.Size = new System.Drawing.Size(98, 22);
            this.textBoxCmdNEC.TabIndex = 10;
            // 
            // buttonSendNEC
            // 
            this.buttonSendNEC.Location = new System.Drawing.Point(15, 321);
            this.buttonSendNEC.Name = "buttonSendNEC";
            this.buttonSendNEC.Size = new System.Drawing.Size(86, 23);
            this.buttonSendNEC.TabIndex = 11;
            this.buttonSendNEC.Text = "Send NEC";
            this.buttonSendNEC.UseVisualStyleBackColor = true;
            this.buttonSendNEC.Click += new System.EventHandler(this.buttonSendNEC_Click);
            // 
            // AVRemoteExplorerForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(879, 563);
            this.Controls.Add(this.buttonSendNEC);
            this.Controls.Add(this.textBoxCmdNEC);
            this.Controls.Add(this.textBoxTargetNEC);
            this.Controls.Add(this.buttonCarrier);
            this.Controls.Add(this.textBoxDutyCycle);
            this.Controls.Add(this.textBoxCarrier);
            this.Controls.Add(this.buttonStatus);
            this.Controls.Add(this.buttonReset);
            this.Controls.Add(this.labelToWrite);
            this.Controls.Add(this.labelToRead);
            this.Controls.Add(this.listBoxInfo);
            this.Name = "AVRemoteExplorerForm";
            this.Text = "AV Remote Explorer";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.IO.Ports.SerialPort serialPortAVR;
        private System.Windows.Forms.ListBox listBoxInfo;
        private System.Windows.Forms.Label labelToRead;
        private System.Windows.Forms.Label labelToWrite;
        private System.Windows.Forms.Timer timerPoll;
        private System.Windows.Forms.Button buttonReset;
        private System.Windows.Forms.Button buttonStatus;
        private System.Windows.Forms.TextBox textBoxCarrier;
        private System.Windows.Forms.TextBox textBoxDutyCycle;
        private System.Windows.Forms.Button buttonCarrier;
        private System.Windows.Forms.TextBox textBoxTargetNEC;
        private System.Windows.Forms.TextBox textBoxCmdNEC;
        private System.Windows.Forms.Button buttonSendNEC;
    }
}

