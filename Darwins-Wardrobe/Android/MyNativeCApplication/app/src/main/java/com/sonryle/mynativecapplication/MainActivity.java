package com.sonryle.mynativecapplication;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.sonryle.mynativecapplication.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Load our C++ library
    static {
        System.loadLibrary("mynativecapplication");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // ().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        /*
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
         */
    }
}