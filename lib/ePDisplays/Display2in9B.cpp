#include "Display2in9B.hpp"
#include "DisplayConnector.hpp"

#include <Arduino.h>
#include <stdlib.h>

namespace Displays {

    Display2in9B::~Display2in9B() {
    };

    Display2in9B::Display2in9B() {
        this->width = EPD_WIDTH;
        this->height = EPD_HEIGHT;
    };

    int Display2in9B::Init(void) {
        /* this calls the peripheral hardware interface, see epdif */
        if (IfInit() != 0) {
            return -1;
        }
        /* EPD hardware init start */
        Reset();
        SendCommand(BOOSTER_SOFT_START);
        SendData(0x17);
        SendData(0x17);
        SendData(0x17);
        SendCommand(POWER_ON);
        WaitUntilIdle();
        SendCommand(PANEL_SETTING);
        SendData(0x8F);
        SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
        SendData(0x77);
        SendCommand(TCON_RESOLUTION);
        SendData(0x80);
        SendData(0x01);
        SendData(0x28);
        SendCommand(VCM_DC_SETTING_REGISTER);
        SendData(0X0A);
        /* EPD hardware init end */
        return 0;

    }

    void Display2in9B::SetFrame(UI::Frame *frame, DisplayColor color = DisplayColor::Black){
        if (color == DisplayColor::Black)
            SetPartialWindowBlack(frame->GetImage(), frame->GetX(), frame->GetY(), frame->GetWidth(), frame->GetHeight()); 
        else if (color == DisplayColor::Colored)
            SetPartialWindowRed(frame->GetImage(), frame->GetX(), frame->GetY(), frame->GetWidth(), frame->GetHeight());   
    }

    /**
     *  @brief: transmit partial data to the SRAM
     */
    void Display2in9B::SetPartialWindow(const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l) {
        SendCommand(PARTIAL_IN);
        SendCommand(PARTIAL_WINDOW);
        SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
        SendData(((x & 0xf8) + w  - 1) | 0x07);
        SendData(y >> 8);        
        SendData(y & 0xff);
        SendData((y + l - 1) >> 8);        
        SendData((y + l - 1) & 0xff);
        SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
        DelayMs(2);
        SendCommand(DATA_START_TRANSMISSION_1);
        if (buffer_black != NULL) {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(buffer_black[i]);  
            }  
        } else {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(0x00);  
            }  
        }
        DelayMs(2);
        SendCommand(DATA_START_TRANSMISSION_2);
        if (buffer_red != NULL) {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(buffer_red[i]);  
            }  
        } else {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(0x00);  
            }  
        }
        DelayMs(2);
        SendCommand(PARTIAL_OUT);  
    }

    /**
     *  @brief: transmit partial data to the black part of SRAM
     */
    void Display2in9B::SetPartialWindowBlack(const unsigned char* buffer_black, int x, int y, int w, int l) {
        SendCommand(PARTIAL_IN);
        SendCommand(PARTIAL_WINDOW);
        SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
        SendData(((x & 0xf8) + w  - 1) | 0x07);
        SendData(y >> 8);        
        SendData(y & 0xff);
        SendData((y + l - 1) >> 8);        
        SendData((y + l - 1) & 0xff);
        SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
        DelayMs(2);
        SendCommand(DATA_START_TRANSMISSION_1);
        if (buffer_black != NULL) {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(buffer_black[i]);  
            }  
        } else {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(0x00);  
            }  
        }
        DelayMs(2);
        SendCommand(PARTIAL_OUT);  
    }

    /**
     *  @brief: transmit partial data to the red part of SRAM
     */
    void Display2in9B::SetPartialWindowRed(const unsigned char* buffer_red, int x, int y, int w, int l) {
        SendCommand(PARTIAL_IN);
        SendCommand(PARTIAL_WINDOW);
        SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
        SendData(((x & 0xf8) + w  - 1) | 0x07);
        SendData(y >> 8);        
        SendData(y & 0xff);
        SendData((y + l - 1) >> 8);        
        SendData((y + l - 1) & 0xff);
        SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
        DelayMs(2);
        SendCommand(DATA_START_TRANSMISSION_2);
        if (buffer_red != NULL) {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(buffer_red[i]);  
            }  
        } else {
            for(int i = 0; i < w  / 8 * l; i++) {
                SendData(0x00);  
            }  
        }
        DelayMs(2);
        SendCommand(PARTIAL_OUT);  
    }

    /**
     * @brief: clear the frame data from the SRAM, this won't refresh the display
     */
    void Display2in9B::ClearFrame(void) {
        SendCommand(TCON_RESOLUTION);
        SendData(width >> 8);
        SendData(width & 0xff);
        SendData(height >> 8);        
        SendData(height & 0xff);

        SendCommand(DATA_START_TRANSMISSION_1);           
        DelayMs(2);
        for(int i = 0; i < width * height / 8; i++) {
            SendData(0xFF);  
        }  
        DelayMs(2);
        SendCommand(DATA_START_TRANSMISSION_2);           
        DelayMs(2);
        for(int i = 0; i < width * height / 8; i++) {
            SendData(0xFF);  
        }  
        DelayMs(2);
    }

    /**
     * @brief: This displays the frame data from SRAM
     */
    void Display2in9B::DisplayFrame(void) {
        SendCommand(DISPLAY_REFRESH); 
        WaitUntilIdle();
    }

    /**
     * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
     *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
     *         check code, the command would be executed if check code = 0xA5. 
     *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
     */
    void Display2in9B::Sleep() {
        SendCommand(DEEP_SLEEP);
        SendData(0xa5);
    }

    void Display2in9B::DebugFrame(UI::Frame *frame){
        if (frame->GetImage() != NULL) {

            Serial.print("Debug frame: ");

            Serial.print("width: ");
            Serial.print(frame->GetWidth());
            Serial.print(", height: ");
            Serial.println(frame->GetHeight());

            for(int i = 0; i < frame->GetWidth()  / 8 * frame->GetHeight(); i++) {
                String str = String(frame->GetImage()[i], BIN); //Convert pixel to byte
                str.replace("1", " "); //Empty char
                str.replace("0", "█"); //Printed char
                Serial.print(str);
        
                if (i != 0 && i % 16 == 0) //Line return
                    Serial.println("|");
            }  
        }
    }

    int Display2in9B::GetWidth(void) {
        return EPD_WIDTH;
    } 
     
    int Display2in9B::GetHeight(void) {
        return EPD_HEIGHT;
    } 

    void Display2in9B::DisplayWindow(Window* window){
        SetFrame(window->RenderBlack(), DisplayColor::Black);
        SetFrame(window->RenderColored(), DisplayColor::Colored);
        DisplayFrame();
    }
}